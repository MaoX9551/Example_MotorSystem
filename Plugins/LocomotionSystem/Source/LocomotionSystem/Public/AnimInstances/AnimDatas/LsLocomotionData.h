// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "Types/LsEnums.h"
#include "LsLocomotionData.generated.h"


/**
 *
 *  基本方向的枚举类型
 *  用于表示角色移动的基本方向
 *
 */
UENUM(Blueprintable)
enum class ECardinalDirection : uint8
{
	EBackward UMETA(DisplayName = "Backward"),
	EForward UMETA(DisplayName = "Forward"),
	ELeft UMETA(DisplayName = "Left"),
	ERight UMETA(DisplayName = "Right"),
};


/**
 *
 * 角色状态结构体
 * 
 */
USTRUCT(BlueprintType)
struct FCharacterState
{
	GENERATED_BODY()


	// 是否正在扫射
	UPROPERTY(BlueprintReadOnly)
	bool bIsStrafing { };

	// 是否正在移动
	UPROPERTY(BlueprintReadOnly)
	bool bIsMoving { };

	// 是否有加速度
	UPROPERTY(BlueprintReadOnly)
	bool bIsAcceleration { };

	// 是否为回转运动
	UPROPERTY(BlueprintReadOnly)
	bool bIsPivoting { };

	// 是否为十字交叉转身
	UPROPERTY(BlueprintReadOnly)
	bool bCrossTurn { };

	// 左脚是否抬起
	UPROPERTY(BlueprintReadOnly)
	bool bLeftFootUp { };

	// 是否下蹲
	UPROPERTY(BlueprintReadOnly)
	bool bIsCrouched { };

	UPROPERTY()
	bool bIsLastFrameCrouched { };

	UPROPERTY(BlueprintReadOnly)
	bool bCrouchStateChanged { };
};


/**
 * 角色移动数据结构体
 * 包含与角色相关的各种向量和速度数据
 */
USTRUCT(BlueprintType)
struct FCharacterMovement
{
	GENERATED_BODY()

	// 地面步态：步行、跑步、 冲刺等
	UPROPERTY(BlueprintReadOnly)
	EGroundGait GroundGait { EGroundGait::ERun };
	// 角色的世界位置
	UPROPERTY(BlueprintReadOnly)
	FVector ActorLocation { };
	// 速度向量3D
	UPROPERTY(BlueprintReadOnly)
	FVector Velocity { };
	// 速度向量2D（忽略Z轴）
	UPROPERTY(BlueprintReadOnly)
	FVector Velocity2D { };
	// 局部空间中的速度向量（2D）
	UPROPERTY(BlueprintReadOnly)
	FVector LocalVelocity2D { };
	// 加速度向量3D
	UPROPERTY(BlueprintReadOnly)
	FVector Acceleration { };
	// 加速度向量2D（忽略Z轴）
	UPROPERTY(BlueprintReadOnly)
	FVector Acceleration2D { };
	// 加速度向量2D（忽略Z轴）
	UPROPERTY(BlueprintReadOnly)
	FVector LocalAcceleration2D { };
	// 最后的非零加速度向量：加速度消失前的加速度向量，用于十字交叉转身
	UPROPERTY(BlueprintReadOnly)
	FVector LastNonZeroAcceleration { };
	// 帧位移量：两帧的位移差量数据
	UPROPERTY(BlueprintReadOnly)
	float FrameDisplacement { };
	// 帧位移速度
	UPROPERTY(BlueprintReadOnly)
	float FrameDisplacementSpeed { };
	// 地面速度
	UPROPERTY(BlueprintReadOnly)
	float GroundSpeed { };
	// 最大地面速度
	UPROPERTY(BlueprintReadOnly)
	float MaxGroundSpeed { };
	
};


/**
 *
 * 角色旋转数据结构体
 * 包含与角色旋转和方向相关的数据
 */
USTRUCT(BlueprintType)
struct FCharacterRotation
{
	GENERATED_BODY()

	// 角色当前旋转
	UPROPERTY(BlueprintReadOnly)
	FRotator ActorRotation { };
    // 角色期望旋转
	UPROPERTY(BlueprintReadOnly)
	FRotator DesireRotation { };
    // 控制器旋转
	UPROPERTY(BlueprintReadOnly)
	FRotator ControlRotation { };
    // 加速度方向旋转
	UPROPERTY(BlueprintReadOnly)
	FRotator AccelerationRotation { };
	// 局部速度方向角度
	UPROPERTY(BlueprintReadOnly)
	float LocalVelocityDirection { };
	// 速度基本方向
	UPROPERTY(BlueprintReadOnly)
	ECardinalDirection VelocityCardinalDirection { };
    // 局部加速度方向角度
	UPROPERTY(BlueprintReadOnly)
	float LocalAccelerationDirection { };
    // 加速度基本方向
	UPROPERTY(BlueprintReadOnly)
	ECardinalDirection AccelerationCardinalDirection { };
    // 帧Yaw偏转角度
	UPROPERTY(BlueprintReadOnly)
	float FrameYaw {  };
	// 帧Yaw偏转速度
	UPROPERTY(BlueprintReadOnly)
	float FrameYawSpeed { };
	// 十字交叉转身角度
	UPROPERTY(BlueprintReadOnly)
	float CrossTurnAngle { };
	
};


/**
 *
 *  运动数据主结构体
 *  整合所有角色运动相关数据的容器结构体
 *
 */
USTRUCT(BlueprintType)
struct FLocomotionData
{
	GENERATED_BODY()

	// 角色状态数据
	UPROPERTY(BlueprintReadOnly)
	FCharacterState States { };
	// 角色移动数据
	UPROPERTY(BlueprintReadOnly)
	FCharacterMovement Movements { };
	// 角色旋转数据
	UPROPERTY(BlueprintReadOnly)
	FCharacterRotation Rotations { };
	// 是否首次更新标识
	UPROPERTY(BlueprintReadOnly)
	bool bIsFirstUpdate { true };
	
};




