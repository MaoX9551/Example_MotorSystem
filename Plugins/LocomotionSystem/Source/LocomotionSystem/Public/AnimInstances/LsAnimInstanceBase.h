// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "LsAnimInstanceBase.generated.h"

class ULsCharacterMovementComponent;


/**
 * 
 */
UCLASS()
class LOCOMOTIONSYSTEM_API ULsAnimInstanceBase : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULsCharacterMovementComponent> MovementComponent;
};
