// OldManUIBase.cpp
#include "OldManUIBase.h"
#include "Character/OldManPersonPlayerController.h"
#include "EventManager/MyEventManager.h"
#include "GlobalEventName.h"

void UOldManUIBase::NativeConstruct()
{
    Super::NativeConstruct();
    // 监听设备切换事件（需要您的 EventManager 支持）
    if (UMyEventManager* EventMgr = UMyEventManager::GetEventManager())
    {
        EventMgr->RegisterCppEvent(UGlobalEventName::Key_Input_InputDeviceChanged, this, &UOldManUIBase::OnInputDeviceChanged);
    }
}

void UOldManUIBase::NativeDestruct()
{
    if (UMyEventManager* EventMgr = UMyEventManager::GetEventManager())
    {
        EventMgr->UnregisterCppEvent(UGlobalEventName::Key_Input_InputDeviceChanged, this, &UOldManUIBase::OnInputDeviceChanged);
    }
    Super::NativeDestruct();
}

void UOldManUIBase::InternalShowUI(UObject* Data)
{
    Super::InternalShowUI(Data);
    // 显示 UI 时，根据当前设备类型启用手柄光标
    AOldManPersonPlayerController* PC = Cast<AOldManPersonPlayerController>(GetOwningPlayer());
    if (PC && PC->GetCurrentHardwareDeviceType() == EHardwareDevicePrimaryType::Gamepad)
    {
        PC->EnableGamepadCursorMode();
    }
}

void UOldManUIBase::InternalHideUI()
{
    Super::InternalHideUI();
    // 隐藏 UI 时，关闭手柄光标模式
    AOldManPersonPlayerController* PC = Cast<AOldManPersonPlayerController>(GetOwningPlayer());
    if (PC)
    {
        PC->DisableGamepadCursorMode();
    }
}

void UOldManUIBase::OnInputDeviceChanged(EHardwareDevicePrimaryType NewDevice)
{
    AOldManPersonPlayerController* PC = Cast<AOldManPersonPlayerController>(GetOwningPlayer());
    if (!PC) return;

    // 只有当当前 UI 处于可见状态时才响应设备切换
    if (GetVisibility() == ESlateVisibility::Visible)
    {
        if (NewDevice == EHardwareDevicePrimaryType::Gamepad)
        {
            PC->EnableGamepadCursorMode();
        }
        else
        {
            PC->DisableGamepadCursorMode();
        }
    }
}