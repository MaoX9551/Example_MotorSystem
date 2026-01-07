// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CommonPlayerController.generated.h"

/**
 * wan
 */
UCLASS(Config=Game)
class COMMONUISYSTEM_API ACommonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACommonPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	
	virtual void ReceivedPlayer() override;
	virtual void SetPawn(APawn* InPawn) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	
	virtual void OnRep_PlayerState() override;
	
};
