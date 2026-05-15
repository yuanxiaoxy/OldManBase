// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XyCharacter/XyPlayerControllerBase.h"
#include "InputActionValue.h"
#include "OldManPersonPlayerController.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class UOldManCameraComponent;
class UUserWidget;

/**
 * Old Man Character Player Controller - Controller part in 3C structure
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
    virtual void Tick(float DeltaTime) override;

    // ========== Enhanced Input System ==========

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RightMouseAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomInAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomOutAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CameraModeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* EcsAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* GamepadClickAction;

    // Input handling functions
    void HandleMove(const FInputActionValue& Value);
    void HandleMoveCancel(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleJumpStart(const FInputActionValue& Value);
    void HandleJumpStop(const FInputActionValue& Value);
    void HandleStartRunning(const FInputActionValue& Value);
    void HandleStopRunning(const FInputActionValue& Value);
    void HandleAttackStart(const FInputActionValue& Value);
    void HandleAttackStop(const FInputActionValue& Value);
    void HandleRightMouseStart(const FInputActionValue& Value);
    void HandleRightMouseStop(const FInputActionValue& Value);
    void HandleZoomIn(const FInputActionValue& Value);
    void HandleZoomOut(const FInputActionValue& Value);
    void HandleCameraMode(const FInputActionValue& Value);
    void HandleEcsPanel(const FInputActionValue& Value);
    void HandleGamepadClickStart(const FInputActionValue& Value);
    void HandleGamepadClickStop(const FInputActionValue& Value);

    virtual void BindCharacterInputs() override;

    // ========== Camera Control ==========

    UFUNCTION(BlueprintCallable, Category = "Camera")
    UOldManCameraComponent* GetCameraComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ZoomCamera(float Delta);

    virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData) override;
    virtual void HandleCharacterDeath() override;

    void HandleCharacterRespawn();

public:
    // ========== Gamepad Cursor Control ==========

    UPROPERTY(BlueprintReadWrite, Category = "Input|GamepadCursor")
    bool bGamepadCursorMode;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input|GamepadCursor")
    FVector2D GamepadCursorPosition;   // 保留用于兼容旧代码，但实际可能不再使用

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|GamepadCursor")
    float GamepadCursorSpeed;

    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    void EnableGamepadCursorMode();

    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    void DisableGamepadCursorMode();

    void MoveGamepadCursor(const FVector2D& Delta);

    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    bool ShouldUseCursorPosition() const;

    // ===== 旧代码中存在的函数，添加回来 =====
    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    FVector2D GetEffectiveCursorScreenPosition() const;

    virtual void SetInputEnabled(bool bEnabled) override;

private:
    UPROPERTY()
    class AOldManCharacter* CachedOldManCharacter;

    UPROPERTY()
    class UEnhancedInputComponent* EnhancedInputComponent;

    FName CurrentCameraMode;

    void SimulateMouseClick(FKey MouseButton, bool bPressed);

    float LastMouseMoveTime;

    // 设备变化回调（非虚函数，直接实现）
    void OnInputHardwareDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId);
};