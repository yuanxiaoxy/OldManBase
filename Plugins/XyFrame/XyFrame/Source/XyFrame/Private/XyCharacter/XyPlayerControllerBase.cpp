#include "XyCharacter/XyPlayerControllerBase.h"
#include "XyCharacter/XyCharacterBase.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWidget.h"
#include "TimerManager.h"

AXyPlayerControllerBase::AXyPlayerControllerBase()
{
    PrimaryActorTick.bCanEverTick = true;
    bInputEnabled = true;
    MouseSensitivity = 1.0f;
    ControllerSensitivity = 1.0f;
    CachedInputComponent = nullptr;
    CachedXyCharacter = nullptr;
    bCachedCharacterValid = false;
    LastHardwareDeviceType = EHardwareDevicePrimaryType::Unspecified;
}

void AXyPlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;
    RegisterEventListeners();

    if (UInputDeviceSubsystem* InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>())
    {
        InputDeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &AXyPlayerControllerBase::OnInputHardwareDeviceChanged);
        UE_LOG(LogTemp, Log, TEXT("AXyPlayerControllerBase: Bound to InputHardwareDeviceChanged event."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AXyPlayerControllerBase: Failed to get InputDeviceSubsystem."));
    }

    EHardwareDevicePrimaryType InitialType = GetCurrentHardwareDeviceType();
    if (InitialType != EHardwareDevicePrimaryType::Unspecified)
    {
        BroadcastInputDeviceChanged(InitialType);
    }

    UE_LOG(LogTemp, Log, TEXT("XyPlayerControllerBase BeginPlay"));
}

void AXyPlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UInputDeviceSubsystem* InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>())
    {
        InputDeviceSubsystem->OnInputHardwareDeviceChanged.RemoveAll(this);
    }
    UnregisterEventListeners();
    Super::EndPlay(EndPlayReason);
}

void AXyPlayerControllerBase::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    CachedXyCharacter = Cast<AXyCharacterBase>(InPawn);
    bCachedCharacterValid = IsValid(CachedXyCharacter);
    BindCharacterInputs();
}

void AXyPlayerControllerBase::OnUnPossess()
{
    CachedXyCharacter = nullptr;
    bCachedCharacterValid = false;
    Super::OnUnPossess();
}

void AXyPlayerControllerBase::SetupInputComponent()
{
    Super::SetupInputComponent();
    CachedInputComponent = InputComponent;
    BindCharacterInputs();
}

void AXyPlayerControllerBase::BindCharacterInputs()
{
    if (!CachedInputComponent) return;
    UE_LOG(LogTemp, Log, TEXT("Character inputs bound"));
}

void AXyPlayerControllerBase::SetInputEnabled(bool bEnabled)
{
    bInputEnabled = bEnabled;
    if (APawn* TempPawn = GetPawn())
    {
        if (bEnabled) TempPawn->EnableInput(this);
        else TempPawn->DisableInput(this);
    }
}

AXyCharacterBase* AXyPlayerControllerBase::GetXyCharacter() const
{
    if (IsValid(CachedXyCharacter)) return CachedXyCharacter;
    CachedXyCharacter = Cast<AXyCharacterBase>(GetPawn());
    return CachedXyCharacter;
}

void AXyPlayerControllerBase::RespawnCharacter()
{
    if (GetXyCharacter() && !GetXyCharacter()->IsAlive()) {}
}

void AXyPlayerControllerBase::RegisterEventListeners()
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr) {}
}

void AXyPlayerControllerBase::UnregisterEventListeners()
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr) {}
}

void AXyPlayerControllerBase::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    switch (EventType)
    {
    case EGameEventType::PlayerDied: HandleCharacterDeath(); break;
    case EGameEventType::PlayerSpawned: break;
    case EGameEventType::ItemCollected: break;
    default: break;
    }
}

void AXyPlayerControllerBase::HandleCharacterDeath()
{
    SetInputEnabled(false);
}

void AXyPlayerControllerBase::SetUIInputMode(EUIInputMode NewMode, UUserWidget* FocusWidget, bool bShowMouse, bool bEnablePawnInput)
{
    switch (NewMode)
    {
    case EUIInputMode::GameOnly:
    {
        FInputModeGameOnly InputMode;
        SetInputMode(InputMode);
        FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::SetDirectly);
        break;
    }
    case EUIInputMode::UIOnly:
    {
        FInputModeUIOnly InputMode;
        if (FocusWidget) InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
        SetInputMode(InputMode);
        if (FocusWidget)
        {
            TSharedPtr<SWidget> SlateWidget = FocusWidget->GetCachedWidget();
            if (SlateWidget.IsValid())
                FSlateApplication::Get().SetKeyboardFocus(SlateWidget.ToSharedRef(), EFocusCause::SetDirectly);
        }
        break;
    }
    case EUIInputMode::UIAndGame:
    {
        FInputModeGameAndUI InputMode;
        if (FocusWidget) InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        if (FocusWidget)
        {
            TSharedPtr<SWidget> SlateWidget = FocusWidget->GetCachedWidget();
            if (SlateWidget.IsValid())
                FSlateApplication::Get().SetKeyboardFocus(SlateWidget.ToSharedRef(), EFocusCause::SetDirectly);
        }
        break;
    }
    }

    bShowMouseCursor = bShowMouse;

    if (bEnablePawnInput)
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn)
        {
            if (NewMode == EUIInputMode::GameOnly)
                ControlledPawn->EnableInput(this);
            else
                ControlledPawn->DisableInput(this);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("SetUIInputMode - Mode=%s, FocusWidget=%s, ShowMouse=%s, PawnInput=%s"),
        *UEnum::GetValueAsString(NewMode),
        FocusWidget ? *FocusWidget->GetName() : TEXT("None"),
        bShowMouse ? TEXT("Yes") : TEXT("No"),
        (NewMode == EUIInputMode::GameOnly && bEnablePawnInput) ? TEXT("Enabled") : TEXT("Disabled"));
}

EHardwareDevicePrimaryType AXyPlayerControllerBase::GetCurrentHardwareDeviceType() const
{
    if (!GetGameInstance() || !GetGameInstance()->GetEngine()) return EHardwareDevicePrimaryType::Unspecified;
    if (UInputDeviceSubsystem* InputDeviceSubsystem = GetGameInstance()->GetEngine()->GetEngineSubsystem<UInputDeviceSubsystem>())
    {
        if (GetLocalPlayer())
        {
            FPlatformUserId UserId = GetLocalPlayer()->GetPlatformUserId();
            FHardwareDeviceIdentifier DeviceInfo = InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(UserId);
            return DeviceInfo.PrimaryDeviceType;
        }
    }
    return EHardwareDevicePrimaryType::Unspecified;
}

void AXyPlayerControllerBase::OnInputHardwareDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId)
{
    if (!GetLocalPlayer() || GetLocalPlayer()->GetPlatformUserId() != UserId) return;
    EHardwareDevicePrimaryType NewDeviceType = GetCurrentHardwareDeviceType();
    if (NewDeviceType != LastHardwareDeviceType)
    {
        BroadcastInputDeviceChanged(NewDeviceType);
        LastHardwareDeviceType = NewDeviceType;
    }
}

void AXyPlayerControllerBase::BroadcastInputDeviceChanged(EHardwareDevicePrimaryType NewDeviceType)
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr) EventMgr->TriggerCppEvent("Key_Input_InputDeviceChanged", NewDeviceType);
    const UEnum* EnumPtr = StaticEnum<EHardwareDevicePrimaryType>();
    if (EnumPtr) UE_LOG(LogTemp, Log, TEXT("BroadcastInputDeviceChanged - New Device Type: %s"), *EnumPtr->GetNameStringByValue(static_cast<int64>(NewDeviceType)));
}