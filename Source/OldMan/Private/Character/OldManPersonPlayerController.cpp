#include "Character/OldManPersonPlayerController.h"
#include "Character/OldManCharacter.h"
#include "Character/OldManCameraComponent.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"
#include "UIManager/UIManager.h"

// Enhanced Input related headers
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <GlobalEventName.h>

AOldManPersonPlayerController::AOldManPersonPlayerController()
{
    // Initialize variables
    CachedOldManCharacter = nullptr;
    EnhancedInputComponent = nullptr;
    CurrentCameraMode = TEXT("ThirdPerson");

    // Gamepad cursor initialization
    bGamepadCursorMode = false;
    GamepadCursorPosition = FVector2D::ZeroVector;
    GamepadCursorSpeed = 1000.0f;
}

void AOldManPersonPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Set input mode
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    // Register event listeners
    RegisterEventListeners();

    // Initialize cursor position to screen center
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    GamepadCursorPosition = FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
}

void AOldManPersonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Get Enhanced Input component
    EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get EnhancedInputComponent"));
        return;
    }
}

void AOldManPersonPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Cache character pointer
    CachedOldManCharacter = Cast<AOldManCharacter>(InPawn);
}

void AOldManPersonPlayerController::OnUnPossess()
{
    // Remove input mapping context
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }
    }

    // Clear cache
    CachedOldManCharacter = nullptr;
    EnhancedInputComponent = nullptr;

    Super::OnUnPossess();
}

void AOldManPersonPlayerController::BindCharacterInputs()
{
    if (!EnhancedInputComponent)
    {
        EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
        if (!EnhancedInputComponent)
        {
            UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent is null in BindCharacterInputs"));
            return;
        }
    }

    if (!DefaultMappingContext)
    {
        UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is not set in OldManPersonPlayerController"));
        return;
    }

    // Add input mapping context to local player subsystem
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }

    EnhancedInputComponent->ClearActionBindings();

    // Bind movement input actions
    if (MoveAction)
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleMoveCancel);
    }

    if (LookAction)
    {
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleLook);
    }

    // Bind character action inputs
    if (JumpAction)
    {
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleJumpStart);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleJumpStop);
    }

    if (RunAction)
    {
        EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleStartRunning);
        EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleStopRunning);
    }

    if (AttackAction)
    {
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleAttackStart);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleAttackStop);
    }

    if (RightMouseAction)
    {
        EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleRightMouseStart);
        EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleRightMouseStop);

    }

    // Bind camera control input actions
    if (ZoomInAction)
    {
        EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleZoomIn);
    }

    if (ZoomOutAction)
    {
        EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleZoomOut);
    }

    if (CameraModeAction)
    {
        EnhancedInputComponent->BindAction(CameraModeAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleCameraMode);
    }

    if (EcsAction)
    {
        EnhancedInputComponent->BindAction(EcsAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleEcsPanel);
    }

    UE_LOG(LogTemp, Log, TEXT("Enhanced Input bindings setup for OldMan character and camera"));
}

// ========== Movement input handling functions ==========
void AOldManPersonPlayerController::HandleLook(const FInputActionValue& Value)
{
    if (!bInputEnabled)
        return;

    // Get 2D vector input value (mouse/gamepad look input)
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (LookAxisVector.IsNearlyZero())
        return;

    //// Check if in gamepad cursor mode and current device is gamepad
    if (CachedOldManCharacter && GetCurrentHardwareDeviceType() == EHardwareDevicePrimaryType::Gamepad && CachedOldManCharacter->bUseCursorPos)
    {
        MoveGamepadCursor(LookAxisVector);
    }

    UOldManCameraComponent* CameraComp = GetCameraComponent();
    if (!CameraComp)
        return;

    if (CachedOldManCharacter)
    {
        CachedOldManCharacter->HandleMouseLook(LookAxisVector);

        // Handle view rotation, pass input through camera component
        float mouseSensitivity = CachedOldManCharacter ? CachedOldManCharacter->CharacterAttributes->MouseSensitivity : 1.0f;
        CameraComp->SetCameraInput(LookAxisVector.Y * mouseSensitivity, LookAxisVector.X * mouseSensitivity);
    }
}

void AOldManPersonPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    FVector2D InputVal2D = Value.Get<FVector2D>();
    FVector InputVal = FVector(InputVal2D.X, InputVal2D.Y, 0.0f);

    // Set movement input
    CachedOldManCharacter->SetMovementInput(InputVal);
}

void AOldManPersonPlayerController::HandleMoveCancel(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter)
        return;

    // Clear movement input
    CachedOldManCharacter->SetMovementInput(FVector::ZeroVector);
}

void AOldManPersonPlayerController::HandleJumpStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    CachedOldManCharacter->SetJumpInput(true);
}

void AOldManPersonPlayerController::HandleJumpStop(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter)
        return;

    CachedOldManCharacter->SetJumpInput(false);
}

void AOldManPersonPlayerController::HandleAttackStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    FGameEventData tempEventData;
    UMyEventManager::GetInstance()->TriggerEventString(UGlobalEventName::Key_Input_InputAttack.ToString(), tempEventData);
    CachedOldManCharacter->DectedActors();
}

void AOldManPersonPlayerController::HandleAttackStop(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter)
        return;
}

void AOldManPersonPlayerController::HandleRightMouseStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    CachedOldManCharacter->StartRightMousePull();
}

void AOldManPersonPlayerController::HandleRightMouseStop(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    CachedOldManCharacter->StopRightMousePull();
}

void AOldManPersonPlayerController::HandleStartRunning(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
        return;

    CachedOldManCharacter->SetRunning(true);
}

void AOldManPersonPlayerController::HandleStopRunning(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter)
        return;

    CachedOldManCharacter->SetRunning(false);
}

// ========== Camera control input handling functions ==========

void AOldManPersonPlayerController::HandleZoomIn(const FInputActionValue& Value)
{
    if (!bInputEnabled)
        return;

    ZoomCamera(-50.0f); // Zoom in (reduce distance)
}

void AOldManPersonPlayerController::HandleZoomOut(const FInputActionValue& Value)
{
    if (!bInputEnabled)
        return;

    ZoomCamera(50.0f); // Zoom out (increase distance)
}

void AOldManPersonPlayerController::HandleCameraMode(const FInputActionValue& Value)
{
    if (!bInputEnabled)
        return;

    // Switch camera mode
    if (CurrentCameraMode == TEXT("ThirdPerson"))
    {
        SetCameraMode(TEXT("FirstPerson"));
    }
    else if (CurrentCameraMode == TEXT("FirstPerson"))
    {
        SetCameraMode(TEXT("FreeLook"));
    }
    else
    {
        SetCameraMode(TEXT("ThirdPerson"));
    }
}

void AOldManPersonPlayerController::HandleEcsPanel(const FInputActionValue& Value)
{
    UUIManager::GetInstance()->ShowUIByName("PausePanel");
}

// ========== Camera control functions ==========

UOldManCameraComponent* AOldManPersonPlayerController::GetCameraComponent() const
{
    if (CachedOldManCharacter)
    {
        return CachedOldManCharacter->CameraComponent;
    }
    return nullptr;
}

void AOldManPersonPlayerController::ZoomCamera(float Delta)
{
    UOldManCameraComponent* CameraComp = GetCameraComponent();
    if (!CameraComp)
        return;

    // Get current distance and apply delta
    float CurrentDistance = CameraComp->CurCameraDistance;
    float NewDistance = FMath::Clamp(CurrentDistance + Delta, 100.0f, 1000.0f);
    CameraComp->SetCameraDistance(NewDistance);
}

void AOldManPersonPlayerController::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    switch (EventType)
    {
    case EGameEventType::PlayerDied:
        HandleCharacterDeath();
        break;
    case EGameEventType::PlayerSpawned:
        // Handle character spawn
        UE_LOG(LogTemp, Log, TEXT("OldMan character spawned"));
        break;
    default:
        // Handle other custom events
        break;
    }
}

void AOldManPersonPlayerController::HandleCharacterDeath()
{
    // Disable input when character dies
    SetInputEnabled(false);

    UE_LOG(LogTemp, Log, TEXT("OldMan character died, respawning in 3 seconds..."));

    // Set respawn timer
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
    {
        MonoMgr->SetTimeout(3.0f, this, &AOldManPersonPlayerController::HandleCharacterRespawn);
    }
}

void AOldManPersonPlayerController::HandleCharacterRespawn()
{
    // Enable input
    SetInputEnabled(true);

    // Respawn logic can be added here, e.g., respawn character
    UE_LOG(LogTemp, Log, TEXT("Respawning OldMan character..."));
}

// ========== Override SetInputEnabled to clear persistent input states ==========
void AOldManPersonPlayerController::SetInputEnabled(bool bEnabled)
{
    // When disabling input, clear all persistent input states
    if (!bEnabled && bInputEnabled)
    {
        if (CachedOldManCharacter)
        {
            // Clear movement input
            CachedOldManCharacter->SetMovementInput(FVector::ZeroVector);
            // Clear jump input
            CachedOldManCharacter->SetJumpInput(false);
            // Clear running state
            CachedOldManCharacter->SetRunning(false);
            // Stop right mouse pull (if character is pulling)
            CachedOldManCharacter->StopRightMousePull(); // This function should handle idempotency internally
            // Attack is a one-shot action, no need to stop
        }
    }

    // Call base class to actually enable/disable pawn input
    Super::SetInputEnabled(bEnabled);
}

// ========== Gamepad cursor control function implementations ==========

void AOldManPersonPlayerController::MoveGamepadCursor(const FVector2D& Delta)
{
    // Update cursor position and clamp within viewport bounds
    GamepadCursorPosition += Delta * GamepadCursorSpeed;
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    GamepadCursorPosition.X = FMath::Clamp(GamepadCursorPosition.X, 0.0f, (float)ViewportSizeX);
    GamepadCursorPosition.Y = FMath::Clamp(GamepadCursorPosition.Y, 0.0f, (float)ViewportSizeY);

    // Move system mouse to virtual cursor position (sync positions)
    SetMouseLocation(GamepadCursorPosition.X, GamepadCursorPosition.Y);
}

// ===== Modified: Get effective cursor screen position, return screen center when cursor hidden =====
FVector2D AOldManPersonPlayerController::GetEffectiveCursorScreenPosition() const
{
    // If in gamepad cursor mode and current device is gamepad, return virtual cursor position
    if (ShouldUseCursorPosition())
    {
        return GamepadCursorPosition;
    }

    // Otherwise, if mouse cursor is visible, return system mouse position
    if (bShowMouseCursor)
    {
        float MouseX, MouseY;
        if (GetMousePosition(MouseX, MouseY))
        {
            return FVector2D(MouseX, MouseY);
        }
    }

    // No valid mouse position (cursor hidden or no mouse), return screen center
    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    return FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
}

bool AOldManPersonPlayerController::ShouldUseCursorPosition() const
{
    // When gamepad cursor mode is enabled and current input device is gamepad, use virtual cursor position
    EHardwareDevicePrimaryType CurrentDevice = GetCurrentHardwareDeviceType();
    return bGamepadCursorMode && CurrentDevice == EHardwareDevicePrimaryType::Gamepad;
}