// @MaoX Coding 


#include "AnimInstances/LsAnimInstanceBase.h"

#include "Components/LsCharacterMovementComponent.h"

void ULsAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!MovementComponent)
	{
		if (TryGetPawnOwner())
		{
			MovementComponent = Cast<ULsCharacterMovementComponent>(TryGetPawnOwner()->GetMovementComponent());
		}
	}
}
