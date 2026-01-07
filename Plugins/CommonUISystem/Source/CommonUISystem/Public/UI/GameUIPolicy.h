// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameUIPolicy.generated.h"


class UCommonLocalPlayer;
class UPrimaryGameLayout;
class UGameUIManagerSubsystem;

/**
 * 多人游戏交互模式
 */
UENUM()
enum class ELocalMultiplayerInteractionMode : uint8
{
	// 仅限主玩家全屏视口，不管其他玩家是否存在
	PrimaryOnly,

	// 单人全屏视口，但玩家可以切换显示和休眠的控制权
	SingleToggle,

	// 同时显示的视口显示给两名玩家
	Simultaneous
};


USTRUCT()
struct FRootViewportLayoutInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UPrimaryGameLayout> RootLayout = nullptr;

	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	FRootViewportLayoutInfo()
	{
	}

	FRootViewportLayoutInfo(ULocalPlayer* InLocalPlayer, UPrimaryGameLayout* InRootLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		  , RootLayout(InRootLayout)
		  , bAddedToViewport(bIsInViewport)
	{
	}

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const { return LocalPlayer == OtherLocalPlayer; }
};


/**
 *  为每一个本地玩家（Local Player）创建并维护一个根布局（Root Layout）
 *  - 它不负责具体的 UI 渲染，而是负责 “决定给谁创建 UI” 以及 “管理 UI 的生命周期”
 *  - Policy（策略） 的作用就是：监听玩家的加入/移除，自动为每个玩家生成一个 Root Layout（根布局），并把这个布局挂载到该玩家的视口上
 *  UGameUIPolicy 的存在是为了解决 UE5 中一个复杂的问题：本地多人游戏（Local Multiplayer / Split-Screen）的 UI 管理。
 *
 *   注意：
 *    Within = GameUIManagerSubsystem
 *    - 这意味着这个对象实例 只能 存在于 GameUIManagerSubsystem 内部。
 *    - 它的 GetOuter() 永远是那个 Subsystem。这保证了策略与管理器的生命周期强绑定
 *  
 */
UCLASS(Abstract, Blueprintable, Within=GameUIManagerSubsystem)
class COMMONUISYSTEM_API UGameUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	template <typename GameUIPolicyClass = UGameUIPolicy>
	static GameUIPolicyClass* GetGameUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<GameUIPolicyClass>(GetGameUIPolicy(WorldContextObject));
	}

	static UGameUIPolicy* GetGameUIPolicy(const UObject* WorldContextObject);

	virtual UWorld* GetWorld() const override;
	UGameUIManagerSubsystem* GetOwningUIManager() const;

	/**
	 * 获取根布局
	 * 作用：这是外界（如你的背包组件、主菜单逻辑）访问 UI 系统的主要入口
	 * 用法：当你想要推入一个菜单时，你不需要自己存变量，而是问 Policy：“嘿，给我也看看当前玩家的根布局在哪？”然后 Policy 查表返回给你。
	 */
	UPrimaryGameLayout* GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const;

	/** 获取分屏交互模式 */
	ELocalMultiplayerInteractionMode GetLocalMultiplayerInteractionMode() const { return LocalMultiplayerInteractionMode; }

	/** 请求主玩家控制权 */
	void RequestPrimaryControl(UPrimaryGameLayout* Layout);

protected:
	// 添加布局到视口
	void AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	// 移除布局从视口
	void RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	// 根布局添加到视口
	virtual void OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	// 根布局从视口移除
	virtual void OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);
	// 根布局释放
	virtual void OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout);

	// 创建布局
	void CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer);
	// 获取布局模板
	TSubclassOf<UPrimaryGameLayout> GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer) const;

private:
	// 分屏交互模式
	ELocalMultiplayerInteractionMode LocalMultiplayerInteractionMode = ELocalMultiplayerInteractionMode::PrimaryOnly;

	// 配置UI模板
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UPrimaryGameLayout> LayoutClass;

	UPROPERTY(Transient)
	TArray<FRootViewportLayoutInfo> RootViewportLayouts;


	/**
	 * 响应玩家加入
	 * 触发时机：当 Subsystem 检测到 GameInstance 有新玩家（LocalPlayer）加入时调用
	 */
	void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	/**
	 * 响应玩家移除
	 * 触发时机：PC 变更 / 逻辑隐藏
	 */
	void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	/**
	 *响应玩家销毁
	 *触发时机：玩家离开游戏
	 */
	void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);


	friend class UGameUIManagerSubsystem;
};
