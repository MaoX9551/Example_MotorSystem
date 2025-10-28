// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"

/**
 *
 * 地面步态枚举
 * 定义角色在地面上的移动步态类型，用于控制移动速度和动画
 *  
 */
UENUM(BlueprintType)
enum class EGroundGait : uint8
{

	EWalk UMETA(DisplayName = "Walk"),
	ERun UMETA(DisplayName = "Run"),
	ESprint UMETA(DisplayName = "Sprint"),
};
