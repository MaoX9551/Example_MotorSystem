// Copyright AssistantGame  2025@ 


#include "PrimaryGameLayout.h"

#include "CommonLocalPlayer.h"
#include "CommonUIExtensions.h"
#include "CommonUISystem.h"
#include "GameUIManagerSubsystem.h"
#include "GameUIPolicy.h"
#include "Kismet/GameplayStatics.h"

UPrimaryGameLayout::UPrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayout(const APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(CastChecked<UCommonLocalPlayer>(PlayerController->GetLocalPlayer())) : nullptr;
}

UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayout(const UCommonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		if (const UGameInstance* GameInstance = LocalPlayer->GetGameInstance())
		{
			if (UGameUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UGameUIManagerSubsystem>())
			{
				if (const UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					return  Policy->GetRootLayout(LocalPlayer);
				}
			}
		}
	}
	return nullptr;
}

void UPrimaryGameLayout::SetIsDormant(bool InDormant)
{
	if (bIsDormant != InDormant)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogCommonUISystem, Display, TEXT("%s PrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = InDormant;
		OnIsDormantChanged();
	}
}

void UPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	for (const auto& Layer : Layers)
	{
		Layer.Value->RemoveWidget(*ActivatableWidget);
	}
}

UCommonActivatableWidgetContainerBase* UPrimaryGameLayout::GetLayerWidget(const FGameplayTag LayerName) const
{
	return Layers.FindRef(LayerName);
}

void UPrimaryGameLayout::OnIsDormantChanged()
{
	// 确定如何处理休眠，过去我们把休眠视为关闭渲染的一种方式
	//以及当我们强制多名玩家使用单人玩家的玩家视图时，其他本地玩家的视图。

	//if (UCommonLocalPlayer* LocalPlayer = GetOwningLocalPlayer<UCommonLocalPlayer>())
	//{
	//	// When the root layout is dormant, we don't want to render anything from the owner's view either
	//	LocalPlayer->SetIsPlayerViewEnabled(!bIsDormant);
	//}

	//SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);

	//OnLayoutDormancyChanged().Broadcast(bIsDormant);
}

void UPrimaryGameLayout::RegisterLayer(const FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())
	{
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UPrimaryGameLayout::OnWidgetStackTransitioning);
		LayerWidget->SetTransitionDuration(0.0);

		Layers.Add(LayerTag, LayerWidget);
	}
}

void UPrimaryGameLayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		// 获取一个签名，并暂停玩家输入
		const FName SuspendToken = UCommonUIExtensions::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransion"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			// 恢复玩家输入
			const FName SuspendToken = SuspendInputTokens.Pop();
			UCommonUIExtensions::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}
