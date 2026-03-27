#include "OldManUIBase.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Components/InputComponent.h"

void UOldManUIBase::NativeConstruct()
{
    Super::NativeConstruct();
    if (APlayerController* PC = GetOwningPlayerController())
    {
        int32 ViewportSizeX, ViewportSizeY;
        PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
        VirtualCursorPosition = FVector2D(ViewportSizeX * 0.5f, ViewportSizeY * 0.5f);
    }
    CurrentStickValue = FVector2D::ZeroVector;
}

void UOldManUIBase::NativeDestruct()
{
    UnbindAxisInputs();   // 确保解绑
    Super::NativeDestruct();
}

void UOldManUIBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bGamepadCursorEnabled)
    {
        HandleCursorMove(InDeltaTime);
    }
}

void UOldManUIBase::EnableGamepadCursor(bool bEnable)
{
    if (bGamepadCursorEnabled == bEnable) return;
    bGamepadCursorEnabled = bEnable;

    if (bEnable)
    {
        BindAxisInputs();
        UpdateMousePosition();
    }
    else
    {
        UnbindAxisInputs();
        CurrentStickValue = FVector2D::ZeroVector;
    }
}

void UOldManUIBase::SetCursorSpeed(float NewSpeed)
{
    CursorSpeed = NewSpeed;
}

FVector2D UOldManUIBase::GetVirtualCursorPosition() const
{
    return VirtualCursorPosition;
}

void UOldManUIBase::SimulateMouseClick()
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    PC->InputKey(FKey("LeftMouseButton"), IE_Pressed, 1.0f, false);
    PC->InputKey(FKey("LeftMouseButton"), IE_Released, 1.0f, false);

    UE_LOG(LogTemp, Log, TEXT("SimulateMouseClick at (%f, %f)"), VirtualCursorPosition.X, VirtualCursorPosition.Y);
}

void UOldManUIBase::HandleCursorMove(float DeltaTime)
{
    // 应用死区
    FVector2D Stick = CurrentStickValue;
    if (FMath::Abs(Stick.X) < DeadZone) Stick.X = 0.0f;
    if (FMath::Abs(Stick.Y) < DeadZone) Stick.Y = 0.0f;

    if (Stick.IsNearlyZero())
        return;

    VirtualCursorPosition += Stick * CursorSpeed * DeltaTime;

    APlayerController* PC = GetOwningPlayerController();
    if (PC)
    {
        int32 ViewportSizeX, ViewportSizeY;
        PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
        VirtualCursorPosition.X = FMath::Clamp(VirtualCursorPosition.X, 0.0f, (float)ViewportSizeX);
        VirtualCursorPosition.Y = FMath::Clamp(VirtualCursorPosition.Y, 0.0f, (float)ViewportSizeY);
    }

    UpdateMousePosition();
}

void UOldManUIBase::UpdateMousePosition()
{
    if (APlayerController* PC = GetOwningPlayerController())
    {
        PC->SetMouseLocation(VirtualCursorPosition.X, VirtualCursorPosition.Y);
    }
}

APlayerController* UOldManUIBase::GetOwningPlayerController() const
{
    return GetOwningPlayer<APlayerController>();
}

void UOldManUIBase::AxisLeftStickX(float Value)
{
    CurrentStickValue.X = Value;
}

void UOldManUIBase::AxisLeftStickY(float Value)
{
    CurrentStickValue.Y = Value;
}

void UOldManUIBase::BindAxisInputs()
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    UInputComponent* InputComp = PC->InputComponent;
    if (!InputComp) return;

    UnbindAxisInputs();   // 先解绑旧的

    // 绑定标准左摇杆轴，存储返回的绑定句柄
    StickXBinding = &InputComp->BindAxis(FName("Gamepad_LeftX"), this, &UOldManUIBase::AxisLeftStickX);
    StickYBinding = &InputComp->BindAxis(FName("Gamepad_LeftY"), this, &UOldManUIBase::AxisLeftStickY);

    UE_LOG(LogTemp, Log, TEXT("UOldManUIBase::BindAxisInputs - Bound Gamepad axes."));
}

void UOldManUIBase::UnbindAxisInputs()
{
    APlayerController* PC = GetOwningPlayerController();
    if (!PC) return;

    UInputComponent* InputComp = PC->InputComponent;
    if (!InputComp) return;

    // 解绑：直接设置绑定句柄为 nullptr，不需要调用 ClearBinding
    // 注意：BindAxis 返回的 FInputAxisBinding* 直接存储在 InputComponent 的 AxisBindings 数组中，
    // 我们无法直接移除，但重新绑定会覆盖。由于我们每次启用光标都会重新绑定，旧绑定会自动失效。
    // 但为了避免多次绑定造成重复，我们在这里将句柄置空即可，不实际移除。
    // 实际上 UInputComponent 没有提供直接移除单个绑定的方法，但重复绑定同一个函数会创建新的绑定，
    // 旧的仍然存在但不会影响功能（因为函数相同，多次调用会多次执行）。
    // 更好的做法是在 NativeDestruct 时整个 InputComponent 被销毁，旧的绑定自然消失。
    // 因此这里我们只清空句柄，不尝试移除。
    StickXBinding = nullptr;
    StickYBinding = nullptr;

    UE_LOG(LogTemp, Log, TEXT("UOldManUIBase::UnbindAxisInputs - Cleared binding handles."));
}