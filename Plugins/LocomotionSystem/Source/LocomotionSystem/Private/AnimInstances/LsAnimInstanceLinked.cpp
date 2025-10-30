// @MaoX Coding 


#include "AnimInstances/LsAnimInstanceLinked.h"

#include "AnimationStateMachineLibrary.h"
#include "AnimCharacterMovementLibrary.h"
#include "AnimDistanceMatchingLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "SequencePlayerLibrary.h"
#include "AnimInstances/LsAnimInstanceMain.h"
#include "Components/LsCharacterMovementComponent.h"

void ULsAnimInstanceLinked::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);


	if (!MainAnimInstance)
	{
		if (GetOwningComponent())
		{
			MainAnimInstance = Cast<ULsAnimInstanceMain>(GetOwningComponent()->GetAnimInstance());
		}
	}
}

// 将动画节点引用转换为序列播放器
FSequencePlayerReference ConvertToSequencePlayer(const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequencePlayerReference SequencePlayer = USequencePlayerLibrary::ConvertToSequencePlayer(Node, ConversionResult);
	return EAnimNodeReferenceConversionResult::Failed == ConversionResult ? FSequencePlayerReference() : SequencePlayer;
}

FSequenceEvaluatorReference ConvertToSequenceEvaluator(const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult ConversionResult;
	const FSequenceEvaluatorReference SequenceEvaluator = USequenceEvaluatorLibrary::ConvertToSequenceEvaluator(Node, ConversionResult);
	return ConversionResult == EAnimNodeReferenceConversionResult::Failed ? FSequenceEvaluatorReference() : SequenceEvaluator;
}



void CustomRotationMatching(const float RotationAlpha, const float DeltaTime, const float InterpSpeed, const FLocomotionData& LocomotionData, FCustomRotationData& OutCustomRotationData, FCurrentAnimData& OutCurrentAnimData)
{
	// 计算动画期望旋转：根据混合系数缩放目标角度，因为动画只有L90/L180/R90/R180，而目标角度并不是这四个固定角度
	const float AnimDesiredRotation          = FRotator::NormalizeAxis(RotationAlpha * OutCurrentAnimData.TargetAngle);
    // 实时获取当前加速度方向
    const float TargetAccelerationDirection  = LocomotionData.Movements.Acceleration.GetSafeNormal2D().Rotation().Yaw;

	// 平滑插值到加速度方向，避免方向突变，提供自然的旋转过渡
	OutCurrentAnimData.CurrentAccelerationDirection = FRotator::NormalizeAxis(
		FMath::RInterpTo(
			FRotator(0.f, OutCurrentAnimData.CurrentAccelerationDirection, 0.f),
			FRotator(0.f, TargetAccelerationDirection, 0.f),
			DeltaTime,
			InterpSpeed
		).Yaw
	);

	// 计算加速度差异：当前加速度方向与进入动画时的加速度方向差异
	const float AccelerationDifference = FRotator::NormalizeAxis(OutCurrentAnimData.CurrentAccelerationDirection - OutCurrentAnimData.EntryAccelerationDirection);
	// 计算调整后的期望旋转：将加速度变化的影响应用到动画期望旋转上，同时缩放这个差异变化值
	const float DesiredRotationChanged = FRotator::NormalizeAxis(AnimDesiredRotation + AccelerationDifference * RotationAlpha);

	// 输出最总自定义旋转：将计算的旋转值结果加到当前的组件旋转值上
	OutCustomRotationData.CustomRotationYaw = FRotator::NormalizeAxis(DesiredRotationChanged + OutCurrentAnimData.EntryRotationYaw);
}



// 设置空闲状态的动画行为（挂载到OutputPose节点上）
void ULsAnimInstanceLinked::Setup_IdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 在空闲状态下，设置自定义旋转模式为保持当前旋转
	MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EHoldRotation;
}

void ULsAnimInstanceLinked::Setup_StartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 匹配动画序列
	StartAnimData.AnimSequence = GetAnimSequence(MainAnimInstance, StartAnimChooserTable);

	// 获取序列求值器
	const FSequenceEvaluatorReference& SequenceEvaluator = ConvertToSequenceEvaluator(Node);

	// 设置动画序列
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, StartAnimData.AnimSequence);
	// 将动画显式设置为起始位置（第0帧）
	// 确保每次进入该状态时都从动画开头开始播放
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);

	// 定义为动画驱动状态
	MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EAnimRotation;
	// 检查是否满足特定条件来切换为插值旋转模式: 用来避免在向前移动时出现平移现象
	// 条件：速度方向和加速度方向均为正向时，使用插值旋转
	if (ECardinalDirection::EForward == MainAnimInstance->LocomotionData.Rotations.VelocityCardinalDirection)
	{
		if (ECardinalDirection::EForward == MainAnimInstance->LocomotionData.Rotations.AccelerationCardinalDirection)
		{
			MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EInterpRotation;
		}
	}
	
	// 将目标角度设为本地加速度方向，即角色最终要转向本地加速度的那个方向
	StartAnimData.TargetAngle                   = MainAnimInstance->LocomotionData.Rotations.LocalAccelerationDirection;
	// 计算当前2D平面上的加速度方向角度
	StartAnimData.CurrentAccelerationDirection  = MainAnimInstance->LocomotionData.Movements.Acceleration.GetSafeNormal2D().Rotation().Yaw;
	// 记录进入时的加速度方向为初始方向
	StartAnimData.EntryAccelerationDirection    = StartAnimData.CurrentAccelerationDirection;
	// 记录角色进入时的初始Yaw旋转值为当前角色的旋转
	StartAnimData.EntryRotationYaw              = MainAnimInstance->LocomotionData.Rotations.ActorRotation.Yaw;

	// 重置播放速率和步幅适配Alpha数据
	StartAnimData.FirstStepAlpha = 0.f;
	StartAnimData.StrideWarpingAlpha = 0.f;
}

void ULsAnimInstanceLinked::Update_StartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 获取序列求值器
	const FSequenceEvaluatorReference& SequenceEvaluator = ConvertToSequenceEvaluator(Node);

	const float DeltaTime  = Context.GetContext()->GetDeltaTime();
	const float CurrentTime = USequenceEvaluatorLibrary::GetAccumulatedTime(SequenceEvaluator);  // 当前播放动画时间
	const float RotationAlpha = StartAnimData.AnimSequence->EvaluateCurveData(AnimCurveName.RotationAlpha, (double)CurrentTime);

	// 在播放动画的几帧时间里，缩放步幅和动画播放率
	// 步幅混合权重计算：小于0.1s则输出权重为0， 大于0.4s则输出权重为1
	 StartAnimData.FirstStepAlpha = FMath::GetMappedRangeValueClamped(
        FVector2f(0.1f, 0.4f),
        FVector2f(0.f, 1.f),
        CurrentTime
	 );
	StartAnimData.StrideWarpingAlpha = StartAnimData.FirstStepAlpha;

	FVector2D PlayRateClamped = PlayRateDefault;
	// 距离检查：如果累计距离小于2个单位，就扩大播放速率
	if (StartAnimData.AnimSequence->EvaluateCurveData(AnimCurveName.Distance, (double)CurrentTime) < 2.f)
	{
		PlayRateClamped.Y = 2.f;
	}
	// 根据步幅权重插值最终播放速率
	const FVector2D PlayRateFinal = FMath::Lerp(FVector2D(0.5f, 1.5f), PlayRateClamped, StartAnimData.FirstStepAlpha);

	// 根据距离匹配调整动画播放速率
	// 这确保了动画与角色实际运动保持同步
	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
		Context,
		SequenceEvaluator,
		MainAnimInstance->LocomotionData.Movements.FrameDisplacement,  // 当前帧的位置距离，也就是Distance Traveled
		AnimCurveName.Distance,
		PlayRateFinal
	);

	CustomRotationMatching(RotationAlpha, DeltaTime, 10.f, MainAnimInstance->LocomotionData, MovementComponent->CustomRotationData, StartAnimData);
}

// 设置循环状态的动画行为（挂载到OutputPose节点上）
void ULsAnimInstanceLinked::Setup_CycleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 在循环状态下，设置自定义旋转模式为插值旋转
	MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EInterpRotation;
}

// 更新循环动画
void ULsAnimInstanceLinked::Update_CycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 从选择器表中获取合适当前状态的动画序列
	CycleAnimData.AnimSequence = GetAnimSequence(MainAnimInstance, CycleAnimChooserTable);
	// 将当前节点转换为序列播放器
	const FSequencePlayerReference& SequencePlayer = ConvertToSequencePlayer(Node);
	// 使用惯性混合设置动画序列，实现平滑的动画过渡
	// 这是Cycle状态内部动画切换时的惯性化混合，比如跑步循环动画切换到行走循环动画，不是状态间过渡时的惯性化设置
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CycleAnimData.AnimSequence);
	// 虚幻引擎的距离匹配：用于根据角色移动速度动态调整动画播放速率
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, MainAnimInstance->LocomotionData.Movements.FrameDisplacementSpeed);
}

void ULsAnimInstanceLinked::Setup_StopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	StopAnimData.AnimSequence = GetAnimSequence(MainAnimInstance, StopAnimChooserTable);

	const FSequenceEvaluatorReference& SequenceEvaluator = ConvertToSequenceEvaluator(Node);

	// 这里为什么不适用混合呢？因为停止的时候不会出现运动速度变化吗？？？
	USequenceEvaluatorLibrary::SetSequence(SequenceEvaluator, StopAnimData.AnimSequence);
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 1.55f);

	// 设置旋转模式为保持当前旋转
	MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EHoldRotation;
}

void ULsAnimInstanceLinked::Update_StopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	const FSequenceEvaluatorReference& SequenceEvaluator = ConvertToSequenceEvaluator(Node);

	// 检查角色是否没有了加速度
	if (!MainAnimInstance->LocomotionData.States.bIsAcceleration)
	{
		// 判断是否仍在移动？ （没有加速度也是可以移动的，因为有惯性）
		if (MainAnimInstance->LocomotionData.States.bIsMoving)
		{

			const float StopDistance = UAnimCharacterMovementLibrary::PredictGroundMovementStopLocation(
				MovementComponent->Velocity,
				MovementComponent->bUseSeparateBrakingFriction,
				MovementComponent->BrakingFriction,
				MovementComponent->GroundFriction,
				MovementComponent->BrakingFrictionFactor,
				MovementComponent->BrakingDecelerationWalking
			).Length();

			// 如果距离大于5就进行距离匹配，太小就没必要了
			if (StopDistance >= 5.f)
			{
				// 使用距离匹配技术，根据预测的停止距离调整动画播放位置
				UAnimDistanceMatchingLibrary::DistanceMatchToTarget(SequenceEvaluator, StopDistance, AnimCurveName.Distance);
			}
		}
	}

	// 推荐动画时间
	USequenceEvaluatorLibrary::AdvanceTime(Context, SequenceEvaluator, 1.25f);
}

UAnimSequence* ULsAnimInstanceLinked::GetAnimSequence(const UObject* ContextObject, const UChooserTable* AnimChooserTable)
{
	check(AnimChooserTable);
	if (!ContextObject) return nullptr;
	// 使用选择器系统根据上下文对象评估并获取适合的动画序列
	return Cast<UAnimSequence>(UChooserFunctionLibrary::EvaluateChooser(ContextObject, AnimChooserTable, UAnimSequence::StaticClass()));
}
