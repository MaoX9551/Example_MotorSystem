// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LsCharacterMovementComponent.generated.h"



/**
 *
 *  自定义旋转模式枚举
 *
 */
UENUM(BlueprintType)
enum class ECustomRotationMode: uint8
{
	// 保持旋转模式 - 维持当前旋转方向
	EHoldRotation UMETA(DisplayName = "HoldRotation"),
	// 插值过渡模式 - 平滑过渡到目标旋转
	EInterpRotation UMETA(DisplayName = "FreeRotation"),
	// 动画旋转模式 - 又动画系统控制
	EAnimRotation UMETA(DisplayName = "AnimRotation"),
};


/**
 *
 * 自定义旋转数据结构体
 * 用于处理角色的自定义旋转逻辑和相关数据
 *
 *
 */
USTRUCT(BlueprintType)
struct FCustomRotationData
{
	GENERATED_BODY()

	// 是否 启用/关闭 自定义旋转
	UPROPERTY(BlueprintReadOnly)
	bool bUseCustomRotation { };
	// 自定义旋转的偏航角度
	UPROPERTY(BlueprintReadOnly)
	float CustomRotationYaw { };
	// 自定义旋转曲线权重值
	UPROPERTY(BlueprintReadOnly)
	float CustomRotationCurveWeight { };
	// 自定义旋转模式
	UPROPERTY(BlueprintReadOnly)
	ECustomRotationMode CustomRotationMode { };

	// 控制器的旋转
	FRotator ControlRotation { };
	// 加速度旋转
	FRotator AccelerationRotation { };
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LOCOMOTIONSYSTEM_API ULsCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	ULsCharacterMovementComponent();

public:

	// 重写父类：在移动前更新角色状态 ~ 在 PhysicsRotation 之前执行
	virtual void UpdateCharacterStateBeforeMovement(float DeltaTime) override;
	// 重写父类：处理物理旋转逻辑
	virtual void PhysicsRotation(float DeltaTime) override;
	// 自定义旋转的相关数据
	UPROPERTY(BlueprintReadOnly)
	FCustomRotationData CustomRotationData;

private:
	
	void UpdateCustomRotation(float DeltaTime);
	
};
