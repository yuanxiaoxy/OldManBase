// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/OldManPersonPlayerController.h"
#include "Character/OldManCharacter.h"
#include "Character/OldManCameraComponent.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"
#include "UIManager/UIManager.h"
#include "UIManager/UIBase.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <GlobalEventName.h>

AOldManPersonPlayerController::AOldManPersonPlayerController()
{
    CachedOldManCharacter = nullptr;
    EnhancedInputComponent = nullptr;
    CurrentCameraMode = TEXT("ThirdPerson");

    bGamepadCursorMode = false;
    GamepadCursorPosition = FVector2D::ZeroVector;
    GamepadCursorSpeed = 1000.0f;
    LastMouseMoveTime = 0.0f;

    PrimaryActorTick.bCanEverTick = true;
}

void AOldManPersonPlayerController::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    RegisterEventListeners();

    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    FVector2D Center(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
    GamepadCursorPosition = Center;
    SetMouseLocation(Center.X, Center.Y);
}

void AOldManPersonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInputComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get EnhancedInputComponent"));
        return;
    }

    if (GamepadClickAction)
    {
        EnhancedInputComponent->BindAction(GamepadClickAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleGamepadClickStart);
        EnhancedInputComponent->BindAction(GamepadClickAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleGamepadClickStop);
    }
}

void AOldManPersonPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    CachedOldManCharacter = Cast<AOldManCharacter>(InPawn);
}

void AOldManPersonPlayerController::OnUnPossess()
{
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (DefaultMappingContext)
        {
            Subsystem->RemoveMappingContext(DefaultMappingContext);
        }
    }

    CachedOldManCharacter = nullptr;
    EnhancedInputComponent = nullptr;

    Super::OnUnPossess();
}

void AOldManPersonPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bGamepadCursorMode)
    {
        float MouseX, MouseY;
        if (GetMousePosition(MouseX, MouseY))
        {
            static FVector2D LastMousePos(MouseX, MouseY);
            if (!FMath::IsNearlyEqual(LastMousePos.X, MouseX) || !FMath::IsNearlyEqual(LastMousePos.Y, MouseY))
            {
                DisableGamepadCursorMode();
            }
            LastMousePos = FVector2D(MouseX, MouseY);
        }
    }
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

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }

    EnhancedInputComponent->ClearActionBindings();

    if (MoveAction)
    {
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleMove);
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleMoveCancel);
    }

    if (LookAction)
    {
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleLook);
    }

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

// ========== Input Handling ==========

void AOldManPersonPlayerController::HandleLook(const FInputActionValue& Value)
{
    if (!bInputEnabled) return;

    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (LookAxisVector.IsNearlyZero()) return;

    if (bGamepadCursorMode && GetCurrentHardwareDeviceType() == EHardwareDevicePrimaryType::Gamepad)
    {
        MoveGamepadCursor(LookAxisVector);
        return;
    }

    UOldManCameraComponent* CameraComp = GetCameraComponent();
    if (!CameraComp) return;

    if (CachedOldManCharacter)
    {
        CachedOldManCharacter->HandleMouseLook(LookAxisVector);
        float mouseSensitivity = CachedOldManCharacter->CharacterAttributes ? CachedOldManCharacter->CharacterAttributes->MouseSensitivity : 1.0f;
        CameraComp->SetCameraInput(LookAxisVector.Y * mouseSensitivity, LookAxisVector.X * mouseSensitivity);
    }
}

void AOldManPersonPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    FVector2D InputVal2D = Value.Get<FVector2D>();
    FVector InputVal = FVector(InputVal2D.X, InputVal2D.Y, 0.0f);
    CachedOldManCharacter->SetMovementInput(InputVal);
}

void AOldManPersonPlayerController::HandleMoveCancel(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter) return;
    CachedOldManCharacter->SetMovementInput(FVector::ZeroVector);
}

void AOldManPersonPlayerController::HandleJumpStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    CachedOldManCharacter->SetJumpInput(true);
}

void AOldManPersonPlayerController::HandleJumpStop(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter) return;
    CachedOldManCharacter->SetJumpInput(false);
}

void AOldManPersonPlayerController::HandleStartRunning(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    CachedOldManCharacter->SetRunning(true);
}

void AOldManPersonPlayerController::HandleStopRunning(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter) return;
    CachedOldManCharacter->SetRunning(false);
}

void AOldManPersonPlayerController::HandleAttackStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    FGameEventData tempEventData;
    UMyEventManager::GetInstance()->TriggerEventString(UGlobalEventName::Key_Input_InputAttack.ToString(), tempEventData);
    CachedOldManCharacter->DectedActors();
}

void AOldManPersonPlayerController::HandleAttackStop(const FInputActionValue& Value)
{
    // One-shot action
}

void AOldManPersonPlayerController::HandleRightMouseStart(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    CachedOldManCharacter->StartRightMousePull();
}

void AOldManPersonPlayerController::HandleRightMouseStop(const FInputActionValue& Value)
{
    if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive()) return;
    CachedOldManCharacter->StopRightMousePull();
}

void AOldManPersonPlayerController::HandleZoomIn(const FInputActionValue& Value)
{
    if (!bInputEnabled) return;
    ZoomCamera(-50.0f);
}

void AOldManPersonPlayerController::HandleZoomOut(const FInputActionValue& Value)
{
    if (!bInputEnabled) return;
    ZoomCamera(50.0f);
}

void AOldManPersonPlayerController::HandleCameraMode(const FInputActionValue& Value)
{
    if (!bInputEnabled) return;

    if (CurrentCameraMode == TEXT("ThirdPerson"))
        CurrentCameraMode = TEXT("FirstPerson");
    else if (CurrentCameraMode == TEXT("FirstPerson"))
        CurrentCameraMode = TEXT("FreeLook");
    else
        CurrentCameraMode = TEXT("ThirdPerson");
}

void AOldManPersonPlayerController::HandleEcsPanel(const FInputActionValue& Value)
{
    UUIManager::GetInstance()->ShowUIByName("PausePanel");
}

void AOldManPersonPlayerController::HandleGamepadClickStart(const FInputActionValue& Value)
{
    if (!bGamepadCursorMode) return;
    SimulateMouseClick(EKeys::LeftMouseButton, true);
}

void AOldManPersonPlayerController::HandleGamepadClickStop(const FInputActionValue& Value)
{
    if (!bGamepadCursorMode) return;
    SimulateMouseClick(EKeys::LeftMouseButton, false);
}

// ========== Camera Functions ==========

UOldManCameraComponent* AOldManPersonPlayerController::GetCameraComponent() const
{
    if (CachedOldManCharacter)
        return CachedOldManCharacter->CameraComponent;
    return nullptr;
}

void AOldManPersonPlayerController::ZoomCamera(float Delta)
{
    UOldManCameraComponent* CameraComp = GetCameraComponent();
    if (!CameraComp) return;
    float CurrentDistance = CameraComp->CurCameraDistance;
    float NewDistance = FMath::Clamp(CurrentDistance + Delta, 100.0f, 1000.0f);
    CameraComp->SetCameraDistance(NewDistance);
}

// ========== Event Handling ==========

void AOldManPersonPlayerController::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    switch (EventType)
    {
    case EGameEventType::PlayerDied:
        HandleCharacterDeath();
        break;
    case EGameEventType::PlayerSpawned:
        UE_LOG(LogTemp, Log, TEXT("OldMan character spawned"));
        break;
    default:
        break;
    }
}

void AOldManPersonPlayerController::HandleCharacterDeath()
{
    SetInputEnabled(false);
    UE_LOG(LogTemp, Log, TEXT("OldMan character died, respawning in 3 seconds..."));
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
        MonoMgr->SetTimeout(3.0f, this, &AOldManPersonPlayerController::HandleCharacterRespawn);
}

void AOldManPersonPlayerController::HandleCharacterRespawn()
{
    SetInputEnabled(true);
    UE_LOG(LogTemp, Log, TEXT("Respawning OldMan character..."));
}

void AOldManPersonPlayerController::SetInputEnabled(bool bEnabled)
{
    if (!bEnabled && bInputEnabled)
    {
        if (CachedOldManCharacter)
        {
            CachedOldManCharacter->SetMovementInput(FVector::ZeroVector);
            CachedOldManCharacter->SetJumpInput(false);
            CachedOldManCharacter->SetRunning(false);
            CachedOldManCharacter->StopRightMousePull();
        }
    }
    Super::SetInputEnabled(bEnabled);
}

// ========== Gamepad Cursor Functions ==========

void AOldManPersonPlayerController::EnableGamepadCursorMode()
{
    if (bGamepadCursorMode) return;
    if (GetCurrentHardwareDeviceType() != EHardwareDevicePrimaryType::Gamepad) return;

    bGamepadCursorMode = true;
    bShowMouseCursor = true;

    UE_LOG(LogTemp, Log, TEXT("Gamepad Cursor Mode Enabled"));
}

void AOldManPersonPlayerController::DisableGamepadCursorMode()
{
    if (!bGamepadCursorMode) return;

    bGamepadCursorMode = false;

    UUIManager* UIMgr = UUIManager::GetUIManager();
    if (UIMgr)
    {
        UUIBase* ActiveUI = UIMgr->GetCurrentInputActiveUI();
        if (ActiveUI)
            bShowMouseCursor = ActiveUI->ShouldShowMouseCursor();
        else
            bShowMouseCursor = false;
    }
    else
    {
        bShowMouseCursor = false;
    }

    UE_LOG(LogTemp, Log, TEXT("Gamepad Cursor Mode Disabled"));
}

void AOldManPersonPlayerController::MoveGamepadCursor(const FVector2D& Delta)
{
    if (!bGamepadCursorMode) return;

    float DeltaTime = GetWorld()->GetDeltaSeconds();
    FVector2D Movement = Delta * GamepadCursorSpeed * DeltaTime;

    float MouseX, MouseY;
    if (!GetMousePosition(MouseX, MouseY))
    {
        int32 ViewportSizeX, ViewportSizeY;
        GetViewportSize(ViewportSizeX, ViewportSizeY);
        MouseX = ViewportSizeX * 0.5f;
        MouseY = ViewportSizeY * 0.5f;
    }

    int32 ViewportSizeX, ViewportSizeY;
    GetViewportSize(ViewportSizeX, ViewportSizeY);
    float NewX = FMath::Clamp(MouseX + Movement.X, 0.0f, (float)ViewportSizeX);
    float NewY = FMath::Clamp(MouseY + Movement.Y, 0.0f, (float)ViewportSizeY);

    SetMouseLocation(NewX, NewY);
    GamepadCursorPosition = FVector2D(NewX, NewY); // 同步虚拟位置
}

bool AOldManPersonPlayerController::ShouldUseCursorPosition() const
{
    return bGamepadCursorMode && GetCurrentHardwareDeviceType() == EHardwareDevicePrimaryType::Gamepad;
}

// ===== 关键：实现 GetEffectiveCursorScreenPosition（兼容旧代码）=====
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

// ========== SimulateMouseClick with correct UE5 InputKey ==========
void AOldManPersonPlayerController::SimulateMouseClick(FKey MouseButton, bool bPressed)
{
    if (bPressed)
        InputKey(MouseButton, IE_Pressed, 1.0f, false);
    else
        InputKey(MouseButton, IE_Released, 1.0f, false);
}

void AOldManPersonPlayerController::OnInputHardwareDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId)
{
    EHardwareDevicePrimaryType NewDevice = GetCurrentHardwareDeviceType();
    if (NewDevice != EHardwareDevicePrimaryType::Gamepad && NewDevice != EHardwareDevicePrimaryType::Unspecified)
    {
        DisableGamepadCursorMode();
    }
}