// @MaoX Coding 


#include "CommonLocalPlayer.h"

#include "UI/GameUIManagerSubsystem.h"
#include "UI/GameUIPolicy.h"


UCommonLocalPlayer::UCommonLocalPlayer()
	:Super(FObjectInitializer::Get())
{
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerControllerSet(FPlayerControllerSetDelegate::FDelegate Delegate)
{
	if (APlayerController* PC = GetPlayerController(GetWorld()))
	{
		Delegate.Execute(this, PC);
	}
	return OnPlayerControllerSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerStateSet(FPlayerStateSetDelegate::FDelegate Delegate)
{
	if (const APlayerController* PC = GetPlayerController(GetWorld()))
	{
		if (APlayerState* PS = PC->PlayerState)
		{
			Delegate.Execute(this, PS);
		}
	}
	return OnPlayerStateSet.Add(Delegate);
}

FDelegateHandle UCommonLocalPlayer::CallAndRegister_OnPlayerPawnSet(FPlayerPawnSetDelegate::FDelegate Delegate)
{
	if (const APlayerController* PC = GetPlayerController(GetWorld()))
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Delegate.Execute(this , Pawn);
		}
	}
	return OnPlayerPawnSet.Add(Delegate);
}

bool UCommonLocalPlayer::GetProjectionData(FViewport* Viewport, FSceneViewProjectionData& ProjectionData, int32 StereoViewIndex) const
{
	if (!bIsPlayerViewEnabled)
	{
		return false;
	}
	return Super::GetProjectionData(Viewport, ProjectionData, StereoViewIndex);
}

UPrimaryGameLayout* UCommonLocalPlayer::GetRootUILayout() const
{
	if (UGameUIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UGameUIManagerSubsystem>())
	{
		if (const UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
		{
			return Policy->GetRootLayout(this);
		}
	}
	return nullptr;
}
