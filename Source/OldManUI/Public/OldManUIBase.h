// OldManUIBase.h
#pragma once
#include "UIManager/UIBase.h"
#include "OldManUIBase.generated.h"

UCLASS()
class UOldManUIBase : public UUIBase
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // 设备变化时调用
    UFUNCTION()
    void OnInputDeviceChanged(EHardwareDevicePrimaryType NewDevice);

protected:
    virtual void InternalShowUI(UObject* Data = nullptr) override;
    virtual void InternalHideUI() override;
};