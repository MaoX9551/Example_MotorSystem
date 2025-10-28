// @MaoX Coding 


#include "Components/LsCharacterMovementComponent.h"
#include "GameFramework/Character.h"


ULsCharacterMovementComponent::ULsCharacterMovementComponent()
{
}

void ULsCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaTime)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaTime);

	UpdateCustomRotation(DeltaTime);
}

void ULsCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{

	// 如果启用了自定义旋转
	if (CustomRotationData.bUseCustomRotation)
	{
		// 检查必要的旋转配置选项，必须至少启用其中一个
		// 1. 若 bOrientRotationToMovement为true，bUseControllerDesiredRotation为false，则进入速度方向运动模式
		// 2. 若 bOrientRotationToMovement为false，bUseControllerDesiredRotation为true，则进入扫射（靶向）运动模式
		if (!(bOrientRotationToMovement || bUseControllerDesiredRotation)) return;

		// 获取当前组件的旋转
		const FRotator& CurrentRotation = UpdatedComponent->GetComponentRotation();
		// 初始化期望旋转为当前旋转
		FRotator DesiredRotation = CurrentRotation;
		// 将期望旋转的偏航角度设置为自定义旋转角度（-180 ~ 180）
		DesiredRotation.Yaw = FRotator::NormalizeAxis(CustomRotationData.CustomRotationYaw);

		
		// 如果当前偏航角度与期望偏航角度有显著差异（超过0.001度）
		if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, 1E-03))
		{
			// 使用FixedTurn函数平滑过渡到目标角度
			// GetDeltaRotation: 返回在DeltaTime 时间间隔内需要旋转角色的角度
			DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, GetDeltaRotation(DeltaTime).Yaw);
		}

		// 应用旋转变化：移动量为0，只更新旋转，false 表示不启用扫描检测（防止碰撞检测）
		// GetDenormalized: 反标准化函数
		MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation.GetDenormalized(), false);
	}
	else
	{

		// 当未启用自定义旋转时，使用父类逻辑
		Super::PhysicsRotation(DeltaTime);
	}
	
}

void ULsCharacterMovementComponent::UpdateCustomRotation(float DeltaTime)
{
	if (!GetCharacterOwner()->GetMesh()->GetAnimInstance()) return;

	// 获取控制器的期望旋转并标准化为[-180, 180]
	CustomRotationData.ControlRotation = GetCharacterOwner()->GetController()->GetDesiredRotation().GetNormalized();
	// 从动画曲线（Enable_CustomRotation）获取自定义旋转的权重值
	CustomRotationData.CustomRotationCurveWeight = GetCharacterOwner()->GetMesh()->GetAnimInstance()->GetCurveValue(FName("Enable_CustomRotation"));
	
	// 启用自定义旋转的条件：
	// 1.没有根运动源（根运动优先）不能妨碍攻击、交互等蒙太奇的根运动
	// 2.自定义旋转的权重值大于0.99（几乎完全启用）
	CustomRotationData.bUseCustomRotation = !HasRootMotionSources() && CustomRotationData.CustomRotationCurveWeight > 0.99f;

	// 如果当前角色正在移动：有加速度不一定有速度（被挡住时）；有速度肯定在移动的，但不一定有加速度（惯性导致的移动）
	if (GetCurrentAcceleration().SizeSquared() > KINDA_SMALL_NUMBER && Velocity.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		// 计算运动中加速度方向的旋转（归一化加速度向量的旋转）
		CustomRotationData.AccelerationRotation = GetCurrentAcceleration().GetSafeNormal().Rotation();
	}


	//  如果启用了自定义旋转
	if (CustomRotationData.bUseCustomRotation)
	{
		// 根据不同的自定义旋转模式处理旋转
		switch (CustomRotationData.CustomRotationMode)
		{
			case ECustomRotationMode::EHoldRotation:
			{
				//  保持旋转模式：CustomRotationYaw值和根组件的旋转保持一致
				// UpdatedComponent指的是移动时的组件，在这里一般指的是角色胶囊体，可能有别的移动Actor它不使用胶囊体的情况
				CustomRotationData.CustomRotationYaw = UpdatedComponent->GetComponentRotation().GetNormalized().Yaw;
				break;
			}
			case ECustomRotationMode::EInterpRotation:
			{
				// 插值模式：如果有加速度，平滑过渡到加速度方向
				const FRotator& CurrentRotation      = UpdatedComponent->GetComponentRotation().GetNormalized();
				const FRotator& DesiredRotation      = FMath::RInterpTo(CurrentRotation, CustomRotationData.AccelerationRotation, DeltaTime, 10.f);
				CustomRotationData.CustomRotationYaw = DesiredRotation.Yaw;
				break;
			}
			case ECustomRotationMode::EAnimRotation:
			{
				// 动画旋转模式：使用动画系统控制，这里不需要做任何处理
				break;
			}
		}
	}
	else
	{
		// 未启用自定义旋转时，使用当前组件的旋转
		CustomRotationData.CustomRotationYaw = UpdatedComponent->GetComponentRotation().GetNormalized().Yaw;
	}

	
	// 确保旋转角度在0 - 360 范围内
	CustomRotationData.CustomRotationYaw =  FRotator::ClampAxis(CustomRotationData.CustomRotationYaw);
}
