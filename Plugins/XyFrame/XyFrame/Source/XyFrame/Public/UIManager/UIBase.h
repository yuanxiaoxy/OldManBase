// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ProgressBar.h"
#include "UITypes.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "UIBase.generated.h"

class UInputMappingContext;
class UInputAction;

USTRUCT(BlueprintType)
struct FUIControlInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FString ControlPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIControlType ControlType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UWidget* Widget;

    FUIControlInfo()
        : ControlType(EUIControlType::Button)
        , Widget(nullptr)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, const FString&, ControlPath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckBoxChanged, const FString&, ControlPath, bool, IsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSliderValueChanged, const FString&, ControlPath, float, Value);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUIInputAction, UInputAction*, InputAction, EUIInputEvent, InputEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInputModeChanged);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIBaseInitialized);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIBaseShown, UObject*, Data);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIBaseHidden);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIBaseClosed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIBaseDataSet, UObject*, Data);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UUIBase(const FObjectInitializer& ObjectInitializer);

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void ShowUI(UObject* Data = nullptr);

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void HideUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void CloseUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void SetData(UObject* Data);

    UFUNCTION(BlueprintCallable, Category = "UI")
    UButton* GetButton(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    UTextBlock* GetText(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    UImage* GetImage(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    USlider* GetSlider(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    UCheckBox* GetCheckBox(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    UProgressBar* GetProgressBar(const FString& ControlPath) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetText(const FString& ControlPath, const FString& Content);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetImage(const FString& ControlPath, UTexture2D* Texture);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetProgress(const FString& ControlPath, float Progress);

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void SetInputMode(EUIInputMode NewInputMode);

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    EUIInputMode GetInputMode() const { return InputMode; }

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void SetInputMappingContext(UInputMappingContext* NewIMC, int32 InPriority = 0);

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    UInputMappingContext* GetInputMappingContext() const { return InputMappingContext; }

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void ActivateInput();

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void DeactivateInput(bool bReleasePressedKeys = true);

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    bool IsInputActivated() const { return bInputActivated; }

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    bool ShouldShowMouseCursor() const;

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    TArray<UInputAction*> GetAllInputActions() const { return BoundInputActions; }

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Input")
    void OnInputAction(UInputAction* InputAction, EUIInputEvent InputEvent);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Input")
    void OnInputStarted(UInputAction* InputAction);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Input")
    void OnInputTriggered(UInputAction* InputAction);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Input")
    void OnInputCompleted(UInputAction* InputAction);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Input")
    void OnInputCanceled(UInputAction* InputAction);

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnButtonClicked OnButtonClicked;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnCheckBoxChanged OnCheckBoxChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnSliderValueChanged OnSliderValueChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI|Input")
    FOnUIInputAction OnInputActionEvent;

    UPROPERTY(BlueprintAssignable, Category = "UI|Input")
    FOnInputModeChanged OnInputModeChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI|Lifecycle")
    FOnUIBaseInitialized OnUIInitialized;

    UPROPERTY(BlueprintAssignable, Category = "UI|Lifecycle")
    FOnUIBaseShown OnUIShown;

    UPROPERTY(BlueprintAssignable, Category = "UI|Lifecycle")
    FOnUIBaseHidden OnUIHidden;

    UPROPERTY(BlueprintAssignable, Category = "UI|Lifecycle")
    FOnUIBaseClosed OnUIClosed;

    UPROPERTY(BlueprintAssignable, Category = "UI|Lifecycle")
    FOnUIBaseDataSet OnDataSetReceived;

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Lifecycle")
    void OnUIInitialize();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Lifecycle")
    void OnUIShow(UObject* Data);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Lifecycle")
    void OnUIHide();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Lifecycle")
    void OnUIClose();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Lifecycle")
    void OnReceiveData(UObject* Data);

    UFUNCTION(BlueprintCallable, Category = "UI")
    UObject* GetCurrentData() const { return CurrentData; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Input", meta = (AllowPrivateAccess = "true"))
    bool bShowMouseCursorWhenActive;

protected:
    UPROPERTY()
    TMap<FString, FUIControlInfo> ControlDictionary;

    UPROPERTY()
    TMap<UWidget*, FString> WidgetPathMap;

    UPROPERTY()
    UObject* CurrentData;

    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Input", meta = (AllowPrivateAccess = "true"))
    EUIInputMode InputMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* InputMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Input", meta = (AllowPrivateAccess = "true"))
    int32 InputPriority;

    bool bInputActivated;
    EUIInputMode OriginalInputMode;
    bool bOriginalMouseCursorVisible;

    UPROPERTY()
    TArray<UInputAction*> BoundInputActions;

    TMap<FKey, TArray<UInputAction*>> KeyToActionMap;

    UFUNCTION()
    virtual void HandleButtonClick(const FString& ControlPath);

    UFUNCTION()
    virtual void HandleCheckBoxChanged(const FString& ControlPath, bool IsChecked);

    UFUNCTION()
    virtual void HandleSliderValueChanged(const FString& ControlPath, float Value);

    UFUNCTION()
    void HandleButtonClickInternal();

    UFUNCTION()
    void HandleCheckBoxChangedInternal(bool IsChecked);

    UFUNCTION()
    void HandleSliderValueChangedInternal(float Value);

    virtual void ProcessInputAction(const UInputAction* InputAction, EUIInputEvent InputEvent);
    void BuildKeyToActionMap();
    TArray<UInputAction*> GetInputActionsFromIMC();
    void SaveOriginalInputSettings();
    void RestoreOriginalInputSettings();

    void InitializeControls();
    void FindChildControls();
    void RegisterControlEvents();

    FString GetControlPath(UWidget* Widget) const;
    void AddControlToDictionary(const FString& Path, UWidget* Widget, EUIControlType Type);
    FString FindControlPathByWidget(UWidget* Widget) const;
    UWidget* GetCurrentEventSourceWidget() const;

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    void InternalInitialize();
};