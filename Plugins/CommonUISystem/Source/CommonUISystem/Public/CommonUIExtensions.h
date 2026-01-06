// Copyright AssistantGame  2025@ 

#pragma once

#include "CoreMinimal.h"
#include "CommonInputTypeEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonUIExtensions.generated.h"

struct FGameplayTag;
class UCommonActivatableWidget;


/**
 * 这个管理器的目的是用你游戏实际需要创建的软件来替代，所以这个类是抽象的，以防止它被创建。
 * 如果你只需要基本功能，可以先把这个子系统子装进你自己的游戏里。
 */
UCLASS()
class COMMONUISYSTEM_API UCommonUIExtensions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/** 获取当前拥有该 UI 的玩家正在使用的输入类型（鼠标/键盘、手柄、触摸） */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "全局用户界面扩展", meta = (WorldContext = "WidgetContextObject"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	/**
	 * 专门判断玩家是否在使用触摸屏
	 * 场景：如果是触摸，可能需要显示更大的按钮或虚拟摇杆
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "全局用户界面扩展", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	/**
	 * 专门判断玩家是否在使用手柄
	 *  场景：如果是手柄，UI 通常需要自动聚焦（Focus）并显示“按 A 确认”的图标，而不是鼠标光标
	 */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "全局用户界面扩展", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	/**
	 * 在指定的层级（由 FGameplayTag 标识，如 UI.Layer.Menu）上创建一个新的 Widget 并显示出来
	 * @param LocalPlayer 本地用户
	 * @param LayerName  必须是 UI.Layer 分类下的 Gameplay Tag
	 * @param WidgetClass  要创建的 Widget 类（硬引用，会立即加载）
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static UCommonActivatableWidget* PushContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSubclassOf<UCommonActivatableWidget> WidgetClass);

	/**
	 * 与上面类似，但使用的是 软引用（Soft Class Ptr）
	 * 异步加载。如果你的菜单非常大（包含很多纹理），使用这个函数可以防止打开菜单时游戏卡顿。它会先流式加载资产，加载完成后再显示。
	 * @param LocalPlayer 
	 * @param LayerName 必须是 UI.Layer 分类下的 Gameplay Tag
	 * @param WidgetClass 要创建的 Widget 类（软引用，会立即加载）
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static void PushStreamedContentToLayer_ForPlayer(const ULocalPlayer* LocalPlayer, UPARAM(meta = (Categories = "UI.Layer")) FGameplayTag LayerName, UPARAM(meta = (AllowAbstract = false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass);

	/**
	 * 将指定的 Widget 从它所在的层级中移除（出栈）
	 * @param ActivatableWidget 指定的Widget
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static void PopContentFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** 一个简单的辅助函数，安全地将 APlayerController 转换为 ULocalPlayer。CommonUI 的很多底层子系统是挂载在 LocalPlayer 上的，所以经常需要这个转换 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static ULocalPlayer* GetLocalPlayerFromController(APlayerController* PlayerController);

    /** 挂起（暂停）玩家的输入 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);

	static FName SuspendInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendReason);

	/** 恢复玩家输入 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "全局用户界面扩展")
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendToken);

	static void ResumeInputForPlayer(ULocalPlayer* LocalPlayer, FName SuspendToken);

private:
	static int32 InputSuspensions;
};
