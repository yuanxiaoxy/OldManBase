// OldManUIBase.h
#pragma once
#include "UIManager/UIBase.h"
#include "OldManUIBase.generated.h"

UCLASS()
class UOldManUIBase : public UUIBase
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable)
    void GamepadClick(FVector2D position);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool NeedGetGamePadClick = false;

protected:
    UFUNCTION()
    void OnInputDeviceChanged(EHardwareDevicePrimaryType InputDevice);

    UFUNCTION(BlueprintImplementableEvent, Category = "InputDevice")
    void BP_OnInputDeviceChanged(EHardwareDevicePrimaryType InputDevice);

    virtual void InternalShowUI(UObject* Data = nullptr) override;
    virtual void InternalHideUI() override;
};