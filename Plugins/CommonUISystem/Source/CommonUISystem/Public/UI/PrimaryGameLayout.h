// Copyright AssistantGame  2025@ 

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonUIExtensions.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "PrimaryGameLayout.generated.h"


class UCommonLocalPlayer;
class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;
/**
 * UI的异步加载作状态
 */
enum class EAsyncWidgetLayerState : uint8
{
	Canceled,
	Initialize,
	AfterPush
};


/**
 * 游戏界面的主要布局
 * 这个小部件类表示如何为单人玩家布局、推送和显示UI的所有层。
 * 分屏游戏中的每位玩家都会拥有自己的主游戏布局
 */
UCLASS(Abstract, meta = (DisableNativeTick))
class COMMONUISYSTEM_API UPrimaryGameLayout : public UCommonUserWidget
{
	GENERATED_BODY()
	
	
public:
	
	UPrimaryGameLayout(const FObjectInitializer& ObjectInitializer);

	static UPrimaryGameLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	static UPrimaryGameLayout* GetPrimaryGameLayout(const APlayerController* PlayerController);
	static UPrimaryGameLayout* GetPrimaryGameLayout(const UCommonLocalPlayer* LocalPlayer);


	/** 休眠的根布局会被折叠，只响应拥有玩家记录的持久动作 */
	void SetIsDormant(bool InDormant);
	bool IsDormant() const { return bIsDormant; }
	
	
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerName, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EAsyncWidgetLayerState, ActivatableWidgetT*) {});
	}
	
		template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerName, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass, TFunction<void(EAsyncWidgetLayerState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UCommonUIExtensions::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;
		
		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
			[this, LayerName, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

				ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit) {
					StateFunc(EAsyncWidgetLayerState::Initialize, &WidgetToInit);
				});

				StateFunc(EAsyncWidgetLayerState::AfterPush, Widget);
			})
		);

		// 设置一个取消代理，这样当该处理器被取消时，我们可以继续输入。
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this,
			[this, StateFunc, SuspendInputToken]()
			{
				UCommonUIExtensions::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
				StateFunc(EAsyncWidgetLayerState::Canceled, nullptr);
			})
		);

		return StreamingHandle;
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerName, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerName, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerWidget(LayerName))
		{
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}

		return nullptr;
	}

	/** 找到该小部件，如果它存在于任何图层上，然后将其从图层中移除。*/
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);
	
	/** 根据标签获取层 */
	UCommonActivatableWidgetContainerBase* GetLayerWidget(const FGameplayTag LayerName) const;

protected:
	
	virtual void OnIsDormantChanged();

	/** 注册一个可以推送小部件的层 */
	UFUNCTION(BlueprintCallable, Category="Layer")
	void RegisterLayer(UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget);

	// 层堆栈的过渡（控制玩家输入）
	void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);
private:
	bool bIsDormant = false;

	// 让我们记录下所有被暂停的输入令牌，以便多个异步用户界面能够同时加载，并且我们能够正确地进行暂停操作。
	TArray<FName> SuspendInputTokens;
	
	// 主布局的注册图层.
	UPROPERTY(Transient, meta = (Categories = "UI.Layer"))
	TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;
	
};
