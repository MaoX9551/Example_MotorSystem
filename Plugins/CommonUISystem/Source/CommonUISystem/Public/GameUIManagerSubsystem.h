// @MaoX Coding 

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameUIManagerSubsystem.generated.h"

class UCommonLocalPlayer;
class UGameUIPolicy;


UCLASS(Config=Game, DisplayName="UI管理")
class UUIManagerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(config, EditAnywhere, DisplayName="默认UI加载策略")
	TSoftClassPtr<UGameUIPolicy> DefaultUIPolicyClass;
};




/**
 * 管理全局 UI 的生命周期,特别是通过 “UI 策略（UI Policy）” 来创建和管理玩家的根 UI 布局（Root Layout）
 * 
 */
UCLASS(Abstract)
class COMMONUISYSTEM_API UGameUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGameUIManagerSubsystem(){  }

	/** 初始化的时候将UI策略加载到内存中 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	/** 销毁的时候将UI策略从内存中卸载 */
	virtual void Deinitialize() override;
	/** 是否应该创建子系统 */
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	const UGameUIPolicy* GetCurrentUIPolicy() const { return CurrentPolicy; }
	UGameUIPolicy* GetCurrentUIPolicy() { return CurrentPolicy; }

	virtual void NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer);
	virtual void NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer);

protected:
	void SwitchToPolicy(UGameUIPolicy* InPolicy);

private:

	UPROPERTY(Transient)
	TObjectPtr<UGameUIPolicy> CurrentPolicy { nullptr };
};
