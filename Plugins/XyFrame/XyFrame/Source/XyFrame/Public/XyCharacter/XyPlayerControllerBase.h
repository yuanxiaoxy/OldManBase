#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"
#include "UIManager/UIBase.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "XyPlayerControllerBase.generated.h"

UCLASS(Abstract, Blueprintable)
class XYFRAME_API AXyPlayerControllerBase : public APlayerController
{
    GENERATED_BODY()

public:
    AXyPlayerControllerBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void SetupInputComponent() override;

public:
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void BindCharacterInputs();

    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void SetInputEnabled(bool bEnabled);

    /** 设置 UI 输入模式
     * @param NewMode        新的输入模式
     * @param FocusWidget    需要获得焦点的 UI 控件（可为 nullptr）
     * @param bShowMouse     是否显示鼠标光标
     * @param bEnablePawnInput 是否自动控制 Pawn 的输入启用/禁用（模式为 GameOnly 时启用 Pawn 输入，否则禁用）
     */
    UFUNCTION()
    virtual void SetUIInputMode(EUIInputMode NewMode, UUserWidget* FocusWidget, bool bShowMouse, bool bEnablePawnInput = false);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyPlayerController")
    class AXyCharacterBase* GetXyCharacter() const;

    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RespawnCharacter();

    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RegisterEventListeners();

    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void UnregisterEventListeners();

    UFUNCTION(BlueprintPure, Category = "XyPlayerController|InputDevice")
    EHardwareDevicePrimaryType GetCurrentHardwareDeviceType() const;

protected:
    UFUNCTION()
    virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData);

    UFUNCTION()
    virtual void HandleCharacterDeath();

    UFUNCTION()
    virtual void OnInputHardwareDeviceChanged(FPlatformUserId UserId, FInputDeviceId DeviceId);

    void BroadcastInputDeviceChanged(EHardwareDevicePrimaryType NewDeviceType);

    UMyEventManager* GetEventManager() const { return UMyEventManager::GetEventManager(); }
    UMonoManager* GetMonoManager() const { return UMonoManager::GetMonoManager(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    bool bInputEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float MouseSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float ControllerSensitivity;

private:
    UInputComponent* CachedInputComponent;
    UPROPERTY()
    mutable class AXyCharacterBase* CachedXyCharacter;
    mutable bool bCachedCharacterValid;
    EHardwareDevicePrimaryType LastHardwareDeviceType;

    // ========== 输入状态缓存（新增） ==========
    EUIInputMode CachedInputMode;
    TWeakObjectPtr<UUserWidget> CachedFocusWidget;
    bool bCachedShowMouse;
};