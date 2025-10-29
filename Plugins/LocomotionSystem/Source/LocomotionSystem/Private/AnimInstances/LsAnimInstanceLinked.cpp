// @MaoX Coding 


#include "AnimInstances/LsAnimInstanceLinked.h"

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

	// 设置开始时的播放时间
	USequenceEvaluatorLibrary::SetExplicitTime(SequenceEvaluator, 0.f);

	// 设置自定义旋转模式为插值旋转
	MovementComponent->CustomRotationData.CustomRotationMode = ECustomRotationMode::EInterpRotation;
}

void ULsAnimInstanceLinked::Update_StartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	if (!MovementComponent || !MainAnimInstance) return;

	// 获取序列求值器
	const FSequenceEvaluatorReference& SequenceEvaluator = ConvertToSequenceEvaluator(Node);

	// 根据距离匹配调整动画播放速率
	UAnimDistanceMatchingLibrary::AdvanceTimeByDistanceMatching(
		Context,
		SequenceEvaluator,
		MainAnimInstance->LocomotionData.Movements.FrameDisplacement,
		AnimCurveName.Distance
	);
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
	CycAnimData.AnimSequence = GetAnimSequence(MainAnimInstance, CycleAnimChooserTable);
	// 将当前节点转换为序列播放器
	const FSequencePlayerReference& SequencePlayer = ConvertToSequencePlayer(Node);
	// 使用惯性混合设置动画序列，实现平滑的动画过渡
	// 这是Cycle状态内部动画切换时的惯性化混合，比如跑步循环动画切换到行走循环动画，不是状态间过渡时的惯性化设置
	USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CycAnimData.AnimSequence);
	// 虚幻引擎的距离匹配：用于根据角色移动速度动态调整动画播放速率
	UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, MainAnimInstance->LocomotionData.Movements.FrameDisplacementSpeed);
}

UAnimSequence* ULsAnimInstanceLinked::GetAnimSequence(const UObject* ContextObject, const UChooserTable* AnimChooserTable)
{
	check(AnimChooserTable);
	if (!ContextObject) return nullptr;
	// 使用选择器系统根据上下文对象评估并获取适合的动画序列
	return Cast<UAnimSequence>(UChooserFunctionLibrary::EvaluateChooser(ContextObject, AnimChooserTable, UAnimSequence::StaticClass()));
}
