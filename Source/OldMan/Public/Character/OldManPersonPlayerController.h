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
class UUserWidget; // New forward declaration

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

    // ========== Enhanced Input System ==========

    // Input mapping context
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    // Input actions - Movement
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    // Input actions - Character actions
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RunAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* RightMouseAction;

    // Input actions - Camera control
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomInAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ZoomOutAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CameraModeAction;

    // Open UI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* EcsAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* GamepadClickAction;

    // Input handling functions - Movement
    void HandleMove(const FInputActionValue& Value);
    void HandleMoveCancel(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleGamePadClick(const FInputActionValue& Value);

    // Input handling functions - Character actions
    void HandleJumpStart(const FInputActionValue& Value);
    void HandleJumpStop(const FInputActionValue& Value);
    void HandleStartRunning(const FInputActionValue& Value);
    void HandleStopRunning(const FInputActionValue& Value);
    void HandleAttackStart(const FInputActionValue& Value);
    void HandleAttackStop(const FInputActionValue& Value);
    void HandleRightMouseStart(const FInputActionValue& Value);
    void HandleRightMouseStop(const FInputActionValue& Value);

    // Input handling functions - Camera control
    void HandleZoomIn(const FInputActionValue& Value);
    void HandleZoomOut(const FInputActionValue& Value);
    void HandleCameraMode(const FInputActionValue& Value);

    // UI binding function
    void HandleEcsPanel(const FInputActionValue& Value);

    // Bind Enhanced Input
    virtual void BindCharacterInputs() override;

    // ========== Camera Control ==========

    // Get character camera component
    UFUNCTION(BlueprintCallable, Category = "Camera")
    UOldManCameraComponent* GetCameraComponent() const;

    // Camera zoom
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ZoomCamera(float Delta);

    // Override parent class event handling functions
    virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData) override;
    virtual void HandleCharacterDeath() override;

    // Handle character respawn
    void HandleCharacterRespawn();

public: // Public area
    // ========== Gamepad Cursor Control ==========

    /** Whether gamepad cursor mode is enabled (e.g., set to true when UI is opened) */
    UPROPERTY(BlueprintReadWrite, Category = "Input|GamepadCursor")
    bool bGamepadCursorMode;

    /** Current virtual cursor position on screen (origin at top-left corner) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input|GamepadCursor")
    FVector2D GamepadCursorPosition;

    /** Gamepad cursor movement speed (pixels/second) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input|GamepadCursor")
    float GamepadCursorSpeed;

    /** Gamepad cursor UMG widget class (specify the widget to use) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|GamepadCursor")
    TSubclassOf<UUserWidget> GamepadCursorWidgetClass;

    /** Get the current effective cursor screen position (real mouse or virtual gamepad cursor) */
    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    FVector2D GetEffectiveCursorScreenPosition() const;

    /** Move virtual cursor (driven by gamepad input) */
    void MoveGamepadCursor(const FVector2D& Delta);

    /** Check whether cursor position should be used for detection (gamepad cursor mode and current device is gamepad) */
    UFUNCTION(BlueprintCallable, Category = "Input|GamepadCursor")
    bool ShouldUseCursorPosition() const;

    /** Update cursor visibility (hide system cursor, show UMG cursor) */
    void UpdateCursorVisibility();

    // Override SetInputEnabled to clear persistent input states when disabling
    virtual void SetInputEnabled(bool bEnabled) override;

private:
    // Cached character pointer
    UPROPERTY()
    class AOldManCharacter* CachedOldManCharacter;

    // Enhanced Input component
    UPROPERTY()
    class UEnhancedInputComponent* EnhancedInputComponent;

    // Current camera mode
    FName CurrentCameraMode;

    // Gamepad cursor UMG widget instance
    UPROPERTY()
    UUserWidget* GamepadCursorWidget;
};