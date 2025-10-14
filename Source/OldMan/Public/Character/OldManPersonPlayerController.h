// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyCharacter/XyPlayerControllerBase.h"
#include "InputActionValue.h"
#include "OldManPersonPlayerController.generated.h"

// 前向声明
class UInputMappingContext;
class UInputAction;
class UOldManCameraComponent;

/**
 * 老人角色玩家控制器 - 3C结构中的Controller部分
 */
UCLASS()
class OLDMAN_API AOldManPersonPlayerController : public AXyPlayerControllerBase
{
	GENERATED_BODY()

public:
	AOldManPersonPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// ========== Enhanced Input System ==========

	// 输入映射上下文
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// 输入动作 - 移动
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// 输入动作 - 角色动作
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	// 输入动作 - 相机控制
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomInAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomOutAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraModeAction;

	// 输入处理函数 - 移动
	void HandleMove(const FInputActionValue& Value);
	void HandleMoveCancel(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);

	// 输入处理函数 - 角色动作
	void HandleJumpStart(const FInputActionValue& Value);
	void HandleJumpStop(const FInputActionValue& Value);
	void HandleStartRunning(const FInputActionValue& Value);
	void HandleStopRunning(const FInputActionValue& Value);
	void HandleAttackStart(const FInputActionValue& Value);
	void HandleAttackStop(const FInputActionValue& Value);

	// 输入处理函数 - 相机控制
	void HandleZoomIn(const FInputActionValue& Value);
	void HandleZoomOut(const FInputActionValue& Value);
	void HandleCameraMode(const FInputActionValue& Value);

	// 绑定Enhanced Input
	virtual void BindCharacterInputs() override;

	// ========== 相机控制 ==========

	// 获取角色相机组件
	UFUNCTION(BlueprintCallable, Category = "Camera")
	UOldManCameraComponent* GetCameraComponent() const;

	// 设置相机模式
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(FName NewMode);

	// 相机缩放
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ZoomCamera(float Delta);

	// ========== 事件处理 ==========
	virtual void RegisterEventListeners() override;
	virtual void UnregisterEventListeners() override;

	// 重写父类事件处理函数
	virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData) override;
	virtual void HandleCharacterDeath() override;

	// 处理角色重生
	void HandleCharacterRespawn();

private:
	// 缓存的角色指针
	UPROPERTY()
	class AOldManCharacter* CachedOldManCharacter;

	// Enhanced Input组件
	UPROPERTY()
	class UEnhancedInputComponent* EnhancedInputComponent;

	// 当前相机模式
	FName CurrentCameraMode;
};