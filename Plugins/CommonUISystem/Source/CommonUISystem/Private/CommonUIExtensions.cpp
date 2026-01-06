// Copyright AssistantGame  2025@ 


#include "CommonUIExtensions.h"

#include "CommonActivatableWidget.h"
#include "CommonInputSubsystem.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"


int32 UCommonUIExtensions::InputSuspensions = 0;


ECommonInputType UCommonUIExtensions::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType();
		}
	}

	return ECommonInputType::Count;
}

bool UCommonUIExtensions::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
		}
	}
	return false;
}

bool UCommonUIExtensions::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		}
	}
	return false;
}

UCommonActivatableWidget* UCommonUIExtensions::PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(LocalPlayer) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}
	return nullptr;
}

void UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, FGameplayTag LayerName, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
}

void UCommonUIExtensions::PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
}

ULocalPlayer* UCommonUIExtensions::GetLocalPlayerFromController(APlayerController* PlayerController)
{
	if (PlayerController)
	{
		return Cast<ULocalPlayer>(PlayerController->Player);
	}

	return nullptr;
}

FName UCommonUIExtensions::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	 return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);;
}

FName UCommonUIExtensions::SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UCommonUIExtensions::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendToken);
}

void UCommonUIExtensions::ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken)
{
	if (SuspendToken == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, false);
	}
}


