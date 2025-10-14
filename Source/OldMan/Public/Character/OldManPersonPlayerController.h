// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyCharacter/XyPlayerControllerBase.h"
#include "InputActionValue.h"
#include "OldManPersonPlayerController.generated.h"

// ǰ������
class UInputMappingContext;
class UInputAction;
class UOldManCameraComponent;

/**
 * ���˽�ɫ��ҿ����� - 3C�ṹ�е�Controller����
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

	// ����ӳ��������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// ���붯�� - �ƶ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// ���붯�� - ��ɫ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AttackAction;

	// ���붯�� - �������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomInAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomOutAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CameraModeAction;

	// ���봦���� - �ƶ�
	void HandleMove(const FInputActionValue& Value);
	void HandleMoveCancel(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);

	// ���봦���� - ��ɫ����
	void HandleJumpStart(const FInputActionValue& Value);
	void HandleJumpStop(const FInputActionValue& Value);
	void HandleStartRunning(const FInputActionValue& Value);
	void HandleStopRunning(const FInputActionValue& Value);
	void HandleAttackStart(const FInputActionValue& Value);
	void HandleAttackStop(const FInputActionValue& Value);

	// ���봦���� - �������
	void HandleZoomIn(const FInputActionValue& Value);
	void HandleZoomOut(const FInputActionValue& Value);
	void HandleCameraMode(const FInputActionValue& Value);

	// ��Enhanced Input
	virtual void BindCharacterInputs() override;

	// ========== ������� ==========

	// ��ȡ��ɫ������
	UFUNCTION(BlueprintCallable, Category = "Camera")
	UOldManCameraComponent* GetCameraComponent() const;

	// �������ģʽ
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(FName NewMode);

	// �������
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ZoomCamera(float Delta);

	// ========== �¼����� ==========
	virtual void RegisterEventListeners() override;
	virtual void UnregisterEventListeners() override;

	// ��д�����¼�������
	virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData) override;
	virtual void HandleCharacterDeath() override;

	// �����ɫ����
	void HandleCharacterRespawn();

private:
	// ����Ľ�ɫָ��
	UPROPERTY()
	class AOldManCharacter* CachedOldManCharacter;

	// Enhanced Input���
	UPROPERTY()
	class UEnhancedInputComponent* EnhancedInputComponent;

	// ��ǰ���ģʽ
	FName CurrentCameraMode;
};