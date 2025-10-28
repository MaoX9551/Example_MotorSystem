// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "LsAnimInstanceBase.h"
#include "AnimDatas/LsLocomotionData.h"
#include "LsAnimInstanceMain.generated.h"

/**
 * 
 */
UCLASS()
class LOCOMOTIONSYSTEM_API ULsAnimInstanceMain : public ULsAnimInstanceBase
{
	GENERATED_BODY()


public:

	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly)
	FLocomotionData LocomotionData;


private:

	// 更新运动数据：计算和处理角色的移动，旋转等运动相关数据
	void UpdateLocomotionData(float DeltaTime);

	/**
	 * 选择基本方位方向
	 * 根据角度值确定最合适的基本方向
	 * @param CurrentAngle  - 当前角度值
	 * @param CurrentDirection  - 当前方向（默认前方）
	 * @param bUseCurrentDirection - 是否使用当前方向作为参考
	 * @param DeadZone  - 死区范围，避免方向频繁切换
	 * @return 计算得到的基本方位方向
	 */
	ECardinalDirection SelectCardinalDirection(float CurrentAngle, ECardinalDirection CurrentDirection = ECardinalDirection::EForward, bool bUseCurrentDirection = false, float DeadZone = 10.f);
};
