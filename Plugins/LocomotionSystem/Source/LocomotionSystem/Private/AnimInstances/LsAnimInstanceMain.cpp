// @MaoX Coding 


#include "AnimInstances/LsAnimInstanceMain.h"

#include "KismetAnimationLibrary.h"
#include "Components/LsCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void ULsAnimInstanceMain::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	UpdateLocomotionData(DeltaSeconds);
}

void ULsAnimInstanceMain::UpdateLocomotionData(float DeltaTime)
{
	if (!MovementComponent || !MovementComponent->GetCharacterOwner()) return;


#pragma region [Update Rotation Data] ------------------------------------------------------------------------------------------
	LocomotionData.Rotations.FrameYaw          = FRotator::NormalizeAxis(MovementComponent->UpdatedComponent->GetComponentRotation().Yaw - LocomotionData.Rotations.ActorRotation.Yaw); // 当前帧的值 - 上一帧的值
	LocomotionData.Rotations.FrameYawSpeed     = UKismetMathLibrary::SafeDivide(LocomotionData.Rotations.FrameYaw, DeltaTime);
	LocomotionData.Rotations.ActorRotation     = MovementComponent->UpdatedComponent->GetComponentRotation(); // 0~±180
	LocomotionData.Rotations.ControlRotation   = MovementComponent->CustomRotationData.ControlRotation; // 0~±180

	if (LocomotionData.bIsFirstUpdate)
	{
		// 首次更新特殊处理: 避免初始帧的突变值
		LocomotionData.Rotations.FrameYaw      = 0.f;
		LocomotionData.Rotations.FrameYawSpeed = 0.f;
	}
#pragma endregion

#pragma region [Update Location Data] ------------------------------------------------------------------------------------------
	LocomotionData.Movements.FrameDisplacement          = (MovementComponent->GetActorLocation() - LocomotionData.Movements.ActorLocation).Size2D();  // 使用当前位置 - 上一帧的位置
	LocomotionData.Movements.FrameDisplacementSpeed     = UKismetMathLibrary::SafeDivide(LocomotionData.Movements.FrameDisplacement, DeltaTime);
    LocomotionData.Movements.ActorLocation              = MovementComponent->GetActorLocation();
    
    if (LocomotionData.bIsFirstUpdate)    
    {    
    	LocomotionData.Movements.FrameDisplacement      =  0.f;
    	LocomotionData.Movements.FrameDisplacementSpeed =  0.f;
    }
	
#pragma endregion

#pragma region [Update Velocity Data] ------------------------------------------------------------------------------------------
	LocomotionData.Movements.Velocity        = MovementComponent->Velocity;
	LocomotionData.Movements.Velocity2D      = LocomotionData.Movements.Velocity * FVector(1.f, 1.f, 0.f);
	LocomotionData.Movements.LocalVelocity2D = LocomotionData.Rotations.ActorRotation.UnrotateVector(LocomotionData.Movements.Velocity2D); // 世界向量转为本地向量
	LocomotionData.Movements.GroundSpeed     = LocomotionData.Movements.Velocity2D.Length();
	LocomotionData.Movements.MaxGroundSpeed  = MovementComponent->GetMaxSpeed();
	LocomotionData.States.bIsMoving          = LocomotionData.Movements.GroundSpeed > KINDA_SMALL_NUMBER;

	// 如果正在移动
	if (LocomotionData.States.bIsMoving)
	{
		// 计算局部空间中的速度方向角度
		LocomotionData.Rotations.LocalVelocityDirection = UKismetAnimationLibrary::CalculateDirection(
             LocomotionData.Movements.Velocity,
             LocomotionData.Rotations.ActorRotation
		);

		// 选择速度的基本方位方向（带智能方向切换）
		LocomotionData.Rotations.VelocityCardinalDirection = SelectCardinalDirection(
			LocomotionData.Rotations.LocalVelocityDirection,
			LocomotionData.Rotations.VelocityCardinalDirection,
			LocomotionData.States.bIsMoving
		);
	}
	
#pragma endregion

#pragma region [Update Acceleration Data] ------------------------------------------------------------------------------------------
	LocomotionData.Movements.Acceleration        = MovementComponent->GetCurrentAcceleration();
	LocomotionData.Movements.Acceleration2D      = LocomotionData.Movements.Acceleration * FVector(1.f, 1.f, 0.f);
	LocomotionData.Movements.LocalAcceleration2D = LocomotionData.Rotations.ActorRotation.UnrotateVector(LocomotionData.Movements.Acceleration2D);
	LocomotionData.States.bIsAcceleration        = LocomotionData.Movements.Acceleration2D.SizeSquared() > KINDA_SMALL_NUMBER;

	// 如果存在加速度
	if (LocomotionData.States.bIsAcceleration)
	{
		// 计算局部空间中的加速度方向角度
		LocomotionData.Rotations.LocalAccelerationDirection = UKismetAnimationLibrary::CalculateDirection(
			LocomotionData.Movements.Acceleration,
			LocomotionData.Rotations.ActorRotation
		);
         // 选择加速度的基本方向
		LocomotionData.Rotations.AccelerationCardinalDirection = SelectCardinalDirection(
			LocomotionData.Rotations.LocalAccelerationDirection,
			LocomotionData.Rotations.AccelerationCardinalDirection,
			LocomotionData.States.bIsAcceleration
		);
	}
#pragma endregion


	// 所有数据处理完成，标记为非首次更新
	LocomotionData.bIsFirstUpdate = false;

}

ECardinalDirection ULsAnimInstanceMain::SelectCardinalDirection(float CurrentAngle, ECardinalDirection CurrentDirection, bool bUseCurrentDirection, float DeadZone)
{

	// 设计优点：
	// 1.防抖动：死区机制避免在边界角度频繁切换方向
	// 2.方向惯性: 保持当前方向的时间更长，操作感更舒适
	// 3.灵活配置：通过参数可调整系统的敏感度
	// 这是一个典型游戏开发中处理方向输入的优雅解决方案
	
	float FwdDeadZone = DeadZone;
	float BwdDeadZone = DeadZone;
	const float AbsAngle = FMath::Abs(CurrentAngle);

	// 如果使用当前方向，增加对应方向的死区（提供方向保持的惯性）
	// 当使用当前方向时，如果当前已经是前方或后方，就增大对应方向的死区范围，使得方向切换需要更大的角度变化，从而提供方向保持的惯性
	if (bUseCurrentDirection)
	{
		if (ECardinalDirection::EForward == CurrentDirection) FwdDeadZone = DeadZone * 2.f;
		if (ECardinalDirection::EBackward == CurrentDirection) BwdDeadZone = DeadZone * 2.f;
	}

	// 前方区域：0 ± （45 + 前方死区）
	if (AbsAngle <= 45.f + FwdDeadZone) return ECardinalDirection::EForward;
	// 后方区域：180 ± （45 - 后方死区）
	if (AbsAngle >= 135.f - BwdDeadZone) return ECardinalDirection::EBackward;

	// 虚幻口诀：左负右正（左半圆0~负180，右半圆0~正180）
	return CurrentAngle > 0.f ? ECardinalDirection::ERight:ECardinalDirection::ELeft;
}
