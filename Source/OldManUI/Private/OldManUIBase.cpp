// OldManUIBase.cpp
#include "OldManUIBase.h"
#include "Character/OldManPersonPlayerController.h"
#include "Character/OldManCharacter.h"
#include "EventManager/MyEventManager.h"
#include "GlobalEventName.h"
#include "Framework/Application/SlateApplication.h"

void UOldManUIBase::GamepadClick(FVector2D ScreenPosition)
{
    //if (!NeedGetGamePadClick)
    //{
    //    return;
    //}

    //FString Msg = FString::Printf(TEXT("[SimulateClick] 目标屏幕坐标: (%.1f, %.1f)"),
    //    ScreenPosition.X, ScreenPosition.Y);

    //// 输出到控制台日志
    //UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);

    //FSlateApplication& SlateApp = FSlateApplication::Get();

    //// 1️⃣ 准备当前按下的按键集合（按下左键时，左键应在集合中）
    //TSet<FKey> PressedButtons;
    //PressedButtons.Add(EKeys::LeftMouseButton);

    //// 2️⃣ 修饰键状态（这里无 Shift/Ctrl/Alt 等）
    //FModifierKeysState ModifierKeys;

    //// 3️⃣ 构建鼠标按下事件
    //FPointerEvent MouseDownEvent(
    //    0,                      // UserIndex (0 为默认用户)
    //    0,                      // PointerIndex (0 为主鼠标)
    //    ScreenPosition,         // 当前屏幕位置
    //    ScreenPosition,         // 上一帧屏幕位置（模拟点击时与当前位置相同即可）
    //    PressedButtons,         // 当前按下的按键集合
    //    EKeys::LeftMouseButton, // 触发本次事件的按键（左键）
    //    0.0f,                   // 滚轮差值（点击为0）
    //    ModifierKeys
    //);

    //// 4️⃣ 派发按下事件
    //SlateApp.ProcessMouseButtonDownEvent(nullptr, MouseDownEvent);

    //// 5️⃣ 构建鼠标抬起事件（此时按下的按键集合应为空）
    //TSet<FKey> NoPressedButtons;
    //FPointerEvent MouseUpEvent(
    //    0,
    //    0,
    //    ScreenPosition,
    //    ScreenPosition,
    //    NoPressedButtons,       // 抬起时没有按键保持按下
    //    EKeys::LeftMouseButton, // 抬起的按键仍然是左键
    //    0.0f,
    //    ModifierKeys
    //);

    //// 6️⃣ 派发抬起事件
    //SlateApp.ProcessMouseButtonUpEvent(MouseUpEvent);
}

void UOldManUIBase::OnInputDeviceChanged(EHardwareDevicePrimaryType InputDevice)
{
    BP_OnInputDeviceChanged(InputDevice);
}

void UOldManUIBase::InternalShowUI(UObject* Data)
{
    Super::InternalShowUI(Data);

    //UMyEventManager::GetInstance()->RegisterCppEvent<UOldManUIBase, FVector2D>(UGlobalEventName::GetKey_GamePad_Click(), this, &UOldManUIBase::GamepadClick);
    UMyEventManager::GetInstance()->RegisterCppEvent<UOldManUIBase, EHardwareDevicePrimaryType>(UGlobalEventName::Key_Input_InputDeviceChanged, this, &UOldManUIBase::OnInputDeviceChanged);
}

void UOldManUIBase::InternalHideUI()
{
    Super::InternalHideUI();

    //UMyEventManager::GetInstance()->RemoveCppEvent(UGlobalEventName::GetKey_GamePad_Click());
    UMyEventManager::GetInstance()->UnregisterCppEvent<UOldManUIBase, EHardwareDevicePrimaryType>(UGlobalEventName::Key_Input_InputDeviceChanged, this, &UOldManUIBase::OnInputDeviceChanged);
}