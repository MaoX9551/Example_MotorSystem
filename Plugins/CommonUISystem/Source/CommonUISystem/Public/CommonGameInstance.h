// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CommonGameInstance.generated.h"

/**
 * 
 */
UCLASS(Abstract, Config = Game)
class COMMONUISYSTEM_API UCommonGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UCommonGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual int32 AddLocalPlayer(ULocalPlayer* NewPlayer, FPlatformUserId UserId) override;
	virtual bool RemoveLocalPlayer(ULocalPlayer* ExistingPlayer) override;
	virtual void Init() override;
	virtual void ReturnToMainMenu() override;

private:

	/** This is the primary player*/
	TWeakObjectPtr<ULocalPlayer> PrimaryPlayer;
};
