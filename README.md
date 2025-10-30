# Example_MotorSystem
新的移动系统（学习阶段）


## C++ 关键代码片段

- 使用惯性混合设置动画序列，实现平滑的动画过渡

```c++
// 这是Cycle状态内部动画切换时的惯性化混合，比如跑步循环动画切换到行走循环动画，不是状态间过渡时的惯性化设置
USequencePlayerLibrary::SetSequenceWithInertialBlending(Context, SequencePlayer, CycAnimData.AnimSequence);
```

- 根据角色移动速度动态调整动画播放速率

```c++
// 虚幻引擎的距离匹配：用于根据角色移动速度动态调整动画播放速率
UAnimDistanceMatchingLibrary::SetPlayrateToMatchSpeed(SequencePlayer, MainAnimInstance->LocomotionData.Movements.FrameDisplacementSpeed);
```

- 让旋转Yaw以固定速度逐渐逼近目标角度，实现平滑受限的转向
```c++
// 使用FixedTurn函数平滑过渡到目标角度
// GetDeltaRotation: 返回在DeltaTime 时间间隔内需要旋转角色的角度
// FMath::FixedTurn: 让旋转的Yaw（偏航角）以固定速度从当前角度逐步逼近目标角度，实现平滑受限的转向。
DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, GetDeltaRotation(DeltaTime).Yaw);
```

- 使用选择器系统根据上下文对象评估并获取适合的动画序列
```c++
UAnimSequence* ULsAnimInstanceLinked::GetAnimSequence(const UObject* ContextObject, const UChooserTable* AnimChooserTable)
{
	check(AnimChooserTable);
	if (!ContextObject) return nullptr;
	// 使用选择器系统根据上下文对象评估并获取适合的动画序列
	return Cast<UAnimSequence>(UChooserFunctionLibrary::EvaluateChooser(ContextObject, AnimChooserTable, UAnimSequence::StaticClass()));
}
```



## 注意

Update_Start 函数中，使用的自定义旋转有点复杂，需要静心研究一下
