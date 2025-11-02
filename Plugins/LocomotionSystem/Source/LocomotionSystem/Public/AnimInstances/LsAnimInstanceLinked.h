// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "Chooser.h"
#include "LsAnimInstanceBase.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "Types/LsEnums.h"
#include "LsAnimInstanceLinked.generated.h"

enum class ECardinalDirection : uint8;
enum class EGroundGait : uint8;
class ULsAnimInstanceMain;

/**
 *
 *  动画曲线结构：用于自定义配置动画曲线的名称
 * 
 */
USTRUCT(BlueprintType)
struct FAnimCurveName
{
	GENERATED_BODY()

	// 动画的距离曲线：主要用于距离匹配
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Distance{"Distance"};

	// 动画的旋转曲线（0 ~ 1）：0表示没有动画旋转数据，1表示动画旋转数据已经结束
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName RotationAlpha{"RotationAlpha"};
};

/**
 *  当前动画数据结构体
 *  存储当前播放动画相关的数据和状态信息
 */
USTRUCT(BlueprintType)
struct FCurrentAnimData
{
	GENERATED_BODY()

	// 当前播放的动画序列资源引用
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UAnimSequence> AnimSequence {};
    // 进入动画状态时的地面步态
	UPROPERTY(BlueprintReadOnly)
	EGroundGait EntryGroundGait { EGroundGait::ERun };
    // 进入动画状态时的运动方向
	UPROPERTY(BlueprintReadOnly)
	ECardinalDirection EntryCardinalDirection { };
    // 第一步混合权重（动画开始的前几帧）
	UPROPERTY(BlueprintReadOnly)
	float FirstStepAlpha { };
     // 步幅扭曲混合Alpha值，控制方向调整强度
	UPROPERTY(BlueprintReadOnly)
	float StrideWarpingAlpha { };
	// 方向扭曲混合Alpha值，控制方向调整强度
	UPROPERTY(BlueprintReadOnly)
	float OrientationWarpingAlpha { };
	// 目标角度
	UPROPERTY(BlueprintReadOnly)
	float TargetAngle { };
    // 当前的加速方向
	UPROPERTY(BlueprintReadOnly)
	float CurrentAccelerationDirection { };
    // 进入动画状态时的加速方向
	UPROPERTY(BlueprintReadOnly)
	float EntryAccelerationDirection { };
    // 进入动画状态时的旋转角度
	UPROPERTY(BlueprintReadOnly)
	float EntryRotationYaw { };
    // 期望的旋转角度
	UPROPERTY(BlueprintReadOnly)
	float DesiredRotationYaw { };
    // 动画开始播放的位置（时间）
	UPROPERTY(BlueprintReadOnly)
	float AnimStartPosition { };
};


/**
 * 
 */
UCLASS()
class LOCOMOTIONSYSTEM_API ULsAnimInstanceLinked : public ULsAnimInstanceBase
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULsAnimInstanceMain> MainAnimInstance;


	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_IdleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(BlueprintReadOnly)
	FCurrentAnimData StartAnimData;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_StartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Update_StartAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(BlueprintReadOnly)
	FCurrentAnimData CycleAnimData;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_CycleState(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Update_CycleAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	
	UPROPERTY(BlueprintReadOnly)
	FCurrentAnimData StopAnimData;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_StopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Update_StopAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(BlueprintReadOnly)
	FCurrentAnimData PivotAnimData;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_PivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Update_PivotAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UPROPERTY(BlueprintReadOnly)
	FCurrentAnimData PostAnimData;
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Setup_PostAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	void Update_PostAnim(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

private:

	UPROPERTY(EditDefaultsOnly, Category="LsAnim")
	FAnimCurveName AnimCurveName;

	UPROPERTY(EditDefaultsOnly, Category="LsAnim")
	FVector2D PlayRateDefault {0.75f, 1.25f};


	UPROPERTY(EditDefaultsOnly, Category="LsAnim|Anim Chooser Table")
	TObjectPtr<UChooserTable> StartAnimChooserTable;
	
	UPROPERTY(EditDefaultsOnly, Category="LsAnim|Anim Chooser Table")
	TObjectPtr<UChooserTable> CycleAnimChooserTable;

	UPROPERTY(EditDefaultsOnly, Category="LsAnim|Anim Chooser Table")
	TObjectPtr<UChooserTable> StopAnimChooserTable;

	UPROPERTY(EditDefaultsOnly, Category="LsAnim|Anim Chooser Table")
	TObjectPtr<UChooserTable> PivotAnimChooserTable;

	static UAnimSequence* GetAnimSequence(const UObject* ContextObject, const UChooserTable* AnimChooserTable);
};
