// @MaoX Coding 


#include "GameUIPolicy.h"

#include "CommonLocalPlayer.h"
#include "CommonUISystem.h"
#include "Engine/GameInstance.h"
#include "Framework/Application/SlateApplication.h"
#include "GameUIManagerSubsystem.h"
#include "PrimaryGameLayout.h"

UGameUIPolicy* UGameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UGameUIManagerSubsystem* UIManager = UGameInstance::GetSubsystem<UGameUIManagerSubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIPolicy();
			}
		}
	}
	return nullptr;
}

UWorld* UGameUIPolicy::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

UGameUIManagerSubsystem* UGameUIPolicy::GetOwningUIManager() const
{
	return CastChecked<UGameUIManagerSubsystem>(GetOuter());
}

UPrimaryGameLayout* UGameUIPolicy::GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}


void UGameUIPolicy::RequestPrimaryControl(UPrimaryGameLayout* Layout)
{
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UPrimaryGameLayout* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);
				break;
			}
		}
		Layout->SetIsDormant(false);
	}
}

void UGameUIPolicy::AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	UE_LOG(LogCommonUISystem, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnRootLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGameUIPolicy::RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogCommonUISystem, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogCommonUISystem, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UGameUIPolicy::OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// 所以我们的控制器可以在PIE中工作，无需点击视口
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UGameUIPolicy::OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
}

void UGameUIPolicy::OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
}

void UGameUIPolicy::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		TSubclassOf<UPrimaryGameLayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UPrimaryGameLayout* NewLayoutObject = CreateWidget<UPrimaryGameLayout>(PlayerController, LayoutWidgetClass);
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

TSubclassOf<UPrimaryGameLayout> UGameUIPolicy::GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer) const
{
	return LayoutClass.LoadSynchronous();
}

/**
 * 延迟初始化。它等待 PlayerController 准备好之后，才真正去创建或显示 UI。这是 UE5 多人/分屏游戏初始化的最佳实践。
 */
void UGameUIPolicy::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	// 这里使用了一个 Lambda 表达式，绑定到 OnPlayerControllerSet 委托上
	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController)
	{
		// 清理之前的UI状态
		NotifyPlayerRemoved(LocalPlayer);

		// 检查是否已经在记录表（RootViewportLayouts）里了？
		if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
		{
			// 如果已经在表里（可能是重新绑定 PC），直接把现有的 Layout 加回屏幕。
			AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
			LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			// 如果是全新的玩家，创建新的 Layout 实例。
			CreateLayoutWidget(LocalPlayer);
		}
	});
}

/**
 * 核心逻辑：解绑与隐藏。注意这里并没有从 RootViewportLayouts 数组中删除数据，只是把 bAddedToViewport 设为了 false。这意味着 UI 实例还在内存里，只是不显示了
 */
void UGameUIPolicy::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	// 1. 查找玩家的 UI 记录
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{

		// 2. 从视口移除 (RemoveFromParent / RemoveFromViewport)
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		// 3. 处理特殊的“单人切换 (SingleToggle)”多人模式
		// 这是一个很少见的模式：多个玩家轮流控制同一个屏幕，而不是分屏。
		if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			UPrimaryGameLayout* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// 如果移除的是“副玩家”，且他的 UI 是激活的：
				// 将副玩家 UI 设为休眠 (Dormant)
				RootLayout->SetIsDormant(true);

				// 找到“主玩家”，唤醒主玩家的 UI
				// 这保证了控制权交还给 Player 1 时，Player 1 的 UI 会重新响应输入
				for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UPrimaryGameLayout* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);
						}
					}
				}
			}
		}
	}
}

/**
 * 核心逻辑：内存清理。这是该玩家 UI 生命周期的终点。
 */
void UGameUIPolicy::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	// 1. 先执行移除逻辑 (从屏幕拿掉)
	NotifyPlayerRemoved(LocalPlayer);

	// 2. 解绑委托 (防止崩溃，因为 Player 马上要没了)
	LocalPlayer->OnPlayerControllerSet.RemoveAll(this);

	// 3. 从管理数组中彻底删除
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UPrimaryGameLayout* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;

		// 从数组移除 struct
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		// 再次确保从视口移除 (双重保险)
		RemoveLayoutFromViewport(LocalPlayer, Layout);

		// 4. 通知销毁
		// 这里通常会调用 Widget 的析构逻辑，或者释放资源
		OnRootLayoutReleased(LocalPlayer, Layout);
	}
}
