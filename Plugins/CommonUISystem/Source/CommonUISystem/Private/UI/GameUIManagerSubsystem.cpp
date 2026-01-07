// @MaoX Coding 


#include "UI/GameUIManagerSubsystem.h"

#include "CommonLocalPlayer.h"
#include "Components/SlateWrapperTypes.h"
#include "GameFramework/HUD.h"
#include "UI/GameUIPolicy.h"
#include "UI/PrimaryGameLayout.h"

void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentPolicy)
	{
	
	    // 1. 获取全局配置单例
    	if (const UUIManagerSettings* Settings = GetDefault<UUIManagerSettings>(); !Settings->DefaultUIPolicyClass.IsNull())
    	{
    		const TSubclassOf<UGameUIPolicy> PolicyClass = Settings->DefaultUIPolicyClass.LoadSynchronous();
    		SwitchToPolicy(NewObject<UGameUIPolicy>(this, PolicyClass));
    	}
	}
	
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UGameUIManagerSubsystem::Tick), 0.0f);
}

void UGameUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();

	SwitchToPolicy(nullptr);
	
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
}

bool UGameUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{

	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 只有在没有其他地方定义覆盖实现的情况下，才创建实例
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UGameUIManagerSubsystem::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
	}
}

void UGameUIManagerSubsystem::SwitchToPolicy(UGameUIPolicy* InPolicy)
{
	if (CurrentPolicy != InPolicy)
	{
		CurrentPolicy = InPolicy;
	}
}

bool UGameUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();
	
	return true;
}

void UGameUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	if (UGameUIPolicy* Policy = GetCurrentUIPolicy())
	{
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			bool bShouldShowUI = true;
			
			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				if (const AHUD* HUD = PC->GetHUD(); HUD && !HUD->bShowHUD)
				{
					bShouldShowUI = false;
				}
			}
			
			if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer)))
			{
				const ESlateVisibility DesiredVisibility = bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
				if (DesiredVisibility != RootLayout->GetVisibility())
				{
					RootLayout->SetVisibility(DesiredVisibility);	
				}
			}
		}
	}
}
