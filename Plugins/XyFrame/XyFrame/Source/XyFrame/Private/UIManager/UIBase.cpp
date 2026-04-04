// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ProgressBar.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Framework/Application/SlateApplication.h"
#include "UIManager/UIManager.h"

UUIBase::UUIBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , CurrentData(nullptr)
    , bIsInitialized(false)
    , InputMode(EUIInputMode::UIOnly)
    , InputMappingContext(nullptr)
    , InputPriority(-1)
    , bShowMouseCursorWhenActive(true)
    , bInputActivated(false)
    , OriginalInputMode(EUIInputMode::GameOnly)
    , bOriginalMouseCursorVisible(false)
{
    bIsFocusable = true;
}

void UUIBase::NativeConstruct()
{
    Super::NativeConstruct();
    InternalInitialize();
}

void UUIBase::NativeDestruct()
{
    Super::NativeDestruct();
    RestoreOriginalInputSettings();
    DeactivateInput(true);
    OnUIClose();
    OnUIClosed.Broadcast();
    ControlDictionary.Empty();
    WidgetPathMap.Empty();
    CurrentData = nullptr;
    BoundInputActions.Empty();
    KeyToActionMap.Empty();
}

void UUIBase::InternalInitialize()
{
    if (bIsInitialized) return;
    InitializeControls();
    bIsInitialized = true;
    SaveOriginalInputSettings();
    OnUIInitialize();
    OnUIInitialized.Broadcast();
}

void UUIBase::InitializeControls()
{
    FindChildControls();
    RegisterControlEvents();
}

void UUIBase::FindChildControls()
{
    if (!WidgetTree) return;
    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);
    for (UWidget* Widget : AllWidgets)
    {
        if (!Widget) continue;
        FString ControlPath = GetControlPath(Widget);
        EUIControlType ControlType = EUIControlType::Button;
        if (Cast<UButton>(Widget))
            ControlType = EUIControlType::Button;
        else if (Cast<UTextBlock>(Widget))
            ControlType = EUIControlType::Text;
        else if (Cast<UImage>(Widget))
            ControlType = EUIControlType::Image;
        else if (Cast<USlider>(Widget))
            ControlType = EUIControlType::Slider;
        else if (Cast<UCheckBox>(Widget))
            ControlType = EUIControlType::CheckBox;
        else if (Cast<UProgressBar>(Widget))
            ControlType = EUIControlType::ProgressBar;
        else
            continue;
        AddControlToDictionary(ControlPath, Widget, ControlType);
        WidgetPathMap.Add(Widget, ControlPath);
    }
}

void UUIBase::RegisterControlEvents()
{
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (UButton* Button = Cast<UButton>(ControlInfo.Widget))
        {
            Button->OnClicked.AddDynamic(this, &UUIBase::HandleButtonClickInternal);
        }
        else if (UCheckBox* CheckBox = Cast<UCheckBox>(ControlInfo.Widget))
        {
            CheckBox->OnCheckStateChanged.AddDynamic(this, &UUIBase::HandleCheckBoxChangedInternal);
        }
        else if (USlider* Slider = Cast<USlider>(ControlInfo.Widget))
        {
            Slider->OnValueChanged.AddDynamic(this, &UUIBase::HandleSliderValueChangedInternal);
        }
    }
}

FString UUIBase::GetControlPath(UWidget* Widget) const
{
    if (!Widget) return FString();
    TArray<FString> PathParts;
    UWidget* CurrentWidget = Widget;
    while (CurrentWidget && CurrentWidget != this)
    {
        PathParts.Insert(CurrentWidget->GetName(), 0);
        CurrentWidget = CurrentWidget->GetParent();
    }
    return FString::Join(PathParts, TEXT("/"));
}

void UUIBase::AddControlToDictionary(const FString& Path, UWidget* Widget, EUIControlType Type)
{
    FUIControlInfo ControlInfo;
    ControlInfo.ControlPath = Path;
    ControlInfo.ControlType = Type;
    ControlInfo.Widget = Widget;
    ControlDictionary.Add(Path, ControlInfo);
}

FString UUIBase::FindControlPathByWidget(UWidget* Widget) const
{
    const FString* FoundPath = WidgetPathMap.Find(Widget);
    return FoundPath ? *FoundPath : FString();
}

UWidget* UUIBase::GetCurrentEventSourceWidget() const
{
    for (const auto& WidgetPathPair : WidgetPathMap)
    {
        UWidget* Widget = WidgetPathPair.Key;
        if (Widget && Widget->IsHovered())
            return Widget;
    }
    return nullptr;
}

// ========== 生命周期方法（委托给 UIManager，支持参数传递） ==========
void UUIBase::ShowUI(UObject* Data)
{
    UUIManager* UIMgr = UUIManager::GetUIManager();
    if (UIMgr)
    {
        UIMgr->ShowUIByWidget(this, Data);
    }
    else
    {
        InternalShowUI(Data);
    }
}

void UUIBase::HideUI(bool bRestorePreviousMainPanel)
{
    UUIManager* UIMgr = UUIManager::GetUIManager();
    if (UIMgr)
    {
        UIMgr->HideUIByWidget(this, bRestorePreviousMainPanel);
    }
    else
    {
        InternalHideUI();
    }
}

void UUIBase::CloseUI(bool bDestroyInstance, bool bRestorePreviousMainPanel)
{
    UUIManager* UIMgr = UUIManager::GetUIManager();
    if (UIMgr)
    {
        UIMgr->CloseUIByWidget(this, bDestroyInstance, bRestorePreviousMainPanel);
    }
    else
    {
        InternalCloseUI();
    }
}

// ========== 内部实现（仅处理视觉和事件，不涉及 UIManager 栈） ==========
void UUIBase::InternalShowUI(UObject* Data)
{
    if (Data) SetData(Data);
    OnUIShow(Data);
    OnUIShown.Broadcast(Data);
}

void UUIBase::InternalHideUI()
{
    SetVisibility(ESlateVisibility::Hidden);
    OnUIHide();
    OnUIHidden.Broadcast();
}

void UUIBase::InternalCloseUI()
{
    RemoveFromParent();
}

void UUIBase::SetData(UObject* Data)
{
    CurrentData = Data;
    OnReceiveData(Data);
    OnDataSetReceived.Broadcast(Data);
}

// ========== 控件获取方法 ==========
UButton* UUIBase::GetButton(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Button)
            return Cast<UButton>(ControlInfo->Widget);
    }
    return nullptr;
}

UTextBlock* UUIBase::GetText(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Text)
            return Cast<UTextBlock>(ControlInfo->Widget);
    }
    return nullptr;
}

UImage* UUIBase::GetImage(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Image)
            return Cast<UImage>(ControlInfo->Widget);
    }
    return nullptr;
}

USlider* UUIBase::GetSlider(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Slider)
            return Cast<USlider>(ControlInfo->Widget);
    }
    return nullptr;
}

UCheckBox* UUIBase::GetCheckBox(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::CheckBox)
            return Cast<UCheckBox>(ControlInfo->Widget);
    }
    return nullptr;
}

UProgressBar* UUIBase::GetProgressBar(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::ProgressBar)
            return Cast<UProgressBar>(ControlInfo->Widget);
    }
    return nullptr;
}

void UUIBase::SetText(const FString& ControlPath, const FString& Content)
{
    if (UTextBlock* TextBlock = GetText(ControlPath))
        TextBlock->SetText(FText::FromString(Content));
}

void UUIBase::SetImage(const FString& ControlPath, UTexture2D* Texture)
{
    if (UImage* Image = GetImage(ControlPath))
        Image->SetBrushFromTexture(Texture);
}

void UUIBase::SetProgress(const FString& ControlPath, float Progress)
{
    if (UProgressBar* ProgressBar = GetProgressBar(ControlPath))
        ProgressBar->SetPercent(Progress);
}

void UUIBase::SetInputMode(EUIInputMode NewInputMode)
{
    if (InputMode != NewInputMode)
    {
        InputMode = NewInputMode;
        OnInputModeChanged.Broadcast();
        UE_LOG(LogTemp, Log, TEXT("UUIBase::SetInputMode - UI: %s, NewMode: %s"), *GetName(), *UEnum::GetValueAsString(InputMode));
    }
}

void UUIBase::SetInputMappingContext(UInputMappingContext* NewIMC, int32 InPriority)
{
    InputMappingContext = NewIMC;
    InputPriority = InPriority;

    if (bInputActivated && InputMappingContext)
    {
        DeactivateInput(false);
        ActivateInput();
    }

    UE_LOG(LogTemp, Log, TEXT("UUIBase::SetInputMappingContext - UI: %s, IMC: %s, Priority: %d"), *GetName(), NewIMC ? *NewIMC->GetName() : TEXT("None"), InPriority);
}

bool UUIBase::ShouldShowMouseCursor() const
{
    if (bShowMouseCursorWhenActive)
        return true;
    return (InputMode == EUIInputMode::UIOnly || InputMode == EUIInputMode::UIAndGame);
}

void UUIBase::ActivateInput()
{
    if (bInputActivated)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIBase::ActivateInput - Already activated for UI: %s"), *GetName());
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIBase::ActivateInput - No PlayerController for UI: %s"), *GetName());
        return;
    }

    if (!InputMappingContext)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIBase::ActivateInput - No InputMappingContext for UI: %s, input will not be processed."), *GetName());
        bInputActivated = true;
        return;
    }

    BuildKeyToActionMap();
    bInputActivated = true;
    SaveOriginalInputSettings();

    UE_LOG(LogTemp, Log, TEXT("UUIBase::ActivateInput - Input activated for UI: %s, IMC: %s"), *GetName(), *InputMappingContext->GetName());
}

void UUIBase::DeactivateInput(bool bReleasePressedKeys)
{
    if (!bInputActivated) return;

    APlayerController* PlayerController = GetOwningPlayer();
    if (PlayerController && bReleasePressedKeys)
    {
        PlayerController->FlushPressedKeys();
    }

    KeyToActionMap.Empty();
    BoundInputActions.Empty();
    bInputActivated = false;
    RestoreOriginalInputSettings();

    UE_LOG(LogTemp, Log, TEXT("UUIBase::DeactivateInput - Input deactivated for UI: %s"), *GetName());
}

void UUIBase::SaveOriginalInputSettings()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController) return;
    bOriginalMouseCursorVisible = PlayerController->bShowMouseCursor;
    OriginalInputMode = InputMode;
}

void UUIBase::RestoreOriginalInputSettings()
{
    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController) return;
    PlayerController->bShowMouseCursor = bOriginalMouseCursorVisible;
}

TArray<UInputAction*> UUIBase::GetInputActionsFromIMC()
{
    TArray<UInputAction*> InputActions;
    if (!InputMappingContext || !IsValid(InputMappingContext)) return InputActions;

    const TArray<FEnhancedActionKeyMapping>& Mappings = InputMappingContext->GetMappings();
    for (const FEnhancedActionKeyMapping& Mapping : Mappings)
    {
        if (Mapping.Action && !InputActions.Contains(Mapping.Action.Get()))
        {
            InputActions.Add(const_cast<UInputAction*>(Mapping.Action.Get()));
        }
    }
    return InputActions;
}

void UUIBase::BuildKeyToActionMap()
{
    KeyToActionMap.Empty();
    BoundInputActions.Empty();

    if (!InputMappingContext || !IsValid(InputMappingContext)) return;

    const TArray<FEnhancedActionKeyMapping>& Mappings = InputMappingContext->GetMappings();
    for (const FEnhancedActionKeyMapping& Mapping : Mappings)
    {
        if (Mapping.Action && Mapping.Key.IsValid())
        {
            UInputAction* Action = const_cast<UInputAction*>(Mapping.Action.Get());
            TArray<UInputAction*>& Actions = KeyToActionMap.FindOrAdd(Mapping.Key);
            Actions.AddUnique(Action);
            BoundInputActions.AddUnique(Action);
        }
    }

    UE_LOG(LogTemp, Verbose, TEXT("UUIBase::BuildKeyToActionMap - Built %d key mappings for UI: %s."), KeyToActionMap.Num(), *GetName());
}

FReply UUIBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (!bInputActivated) return Super::NativeOnKeyDown(InGeometry, InKeyEvent);

    FKey PressedKey = InKeyEvent.GetKey();
    if (const TArray<UInputAction*>* Actions = KeyToActionMap.Find(PressedKey))
    {
        for (UInputAction* Action : *Actions)
        {
            ProcessInputAction(Action, EUIInputEvent::Started);
        }
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UUIBase::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (!bInputActivated) return Super::NativeOnKeyUp(InGeometry, InKeyEvent);

    FKey ReleasedKey = InKeyEvent.GetKey();
    if (const TArray<UInputAction*>* Actions = KeyToActionMap.Find(ReleasedKey))
    {
        for (UInputAction* Action : *Actions)
        {
            ProcessInputAction(Action, EUIInputEvent::Completed);
        }
        return FReply::Handled();
    }

    return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

void UUIBase::ProcessInputAction(const UInputAction* InputAction, EUIInputEvent InputEvent)
{
    if (!InputAction || !IsValid(InputAction)) return;
    UInputAction* NonConstAction = const_cast<UInputAction*>(InputAction);
    OnInputActionEvent.Broadcast(NonConstAction, InputEvent);
    OnInputAction(NonConstAction, InputEvent);
    switch (InputEvent)
    {
    case EUIInputEvent::Started:   OnInputStarted(NonConstAction); break;
    case EUIInputEvent::Triggered: OnInputTriggered(NonConstAction); break;
    case EUIInputEvent::Completed: OnInputCompleted(NonConstAction); break;
    case EUIInputEvent::Canceled:  OnInputCanceled(NonConstAction); break;
    }
}

void UUIBase::HandleButtonClick(const FString& ControlPath)
{
    OnButtonClicked.Broadcast(ControlPath);
}

void UUIBase::HandleCheckBoxChanged(const FString& ControlPath, bool IsChecked)
{
    OnCheckBoxChanged.Broadcast(ControlPath, IsChecked);
}

void UUIBase::HandleSliderValueChanged(const FString& ControlPath, float Value)
{
    OnSliderValueChanged.Broadcast(ControlPath, Value);
}

void UUIBase::HandleButtonClickInternal()
{
    if (UWidget* Sender = GetCurrentEventSourceWidget())
    {
        if (UButton* Button = Cast<UButton>(Sender))
        {
            FString ControlPath = FindControlPathByWidget(Button);
            if (!ControlPath.IsEmpty())
            {
                HandleButtonClick(ControlPath);
                return;
            }
        }
    }
    HandleButtonClick(FString("Unknown"));
}

void UUIBase::HandleCheckBoxChangedInternal(bool IsChecked)
{
    if (UWidget* Sender = GetCurrentEventSourceWidget())
    {
        if (UCheckBox* CheckBox = Cast<UCheckBox>(Sender))
        {
            FString ControlPath = FindControlPathByWidget(CheckBox);
            if (!ControlPath.IsEmpty())
            {
                HandleCheckBoxChanged(ControlPath, IsChecked);
                return;
            }
        }
    }
    HandleCheckBoxChanged(FString("Unknown"), IsChecked);
}

void UUIBase::HandleSliderValueChangedInternal(float Value)
{
    if (UWidget* Sender = GetCurrentEventSourceWidget())
    {
        if (USlider* Slider = Cast<USlider>(Sender))
        {
            FString ControlPath = FindControlPathByWidget(Slider);
            if (!ControlPath.IsEmpty())
            {
                HandleSliderValueChanged(ControlPath, Value);
                return;
            }
        }
    }
    HandleSliderValueChanged(FString("Unknown"), Value);
}