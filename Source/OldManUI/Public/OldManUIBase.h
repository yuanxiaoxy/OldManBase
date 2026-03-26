#pragma once

#include "CoreMinimal.h"
#include "UIManager/UIBase.h"
#include "OldManUIBase.generated.h"

UCLASS(Blueprintable)
class OLDMANUI_API UOldManUIBase : public UUIBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Gamepad Cursor")
    void EnableGamepadCursor(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Gamepad Cursor")
    void SetCursorSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Gamepad Cursor")
    FVector2D GetVirtualCursorPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Gamepad Cursor")
    void SimulateMouseClick();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(EditDefaultsOnly, Category = "Gamepad Cursor")
    float CursorSpeed = 1000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Gamepad Cursor")
    float DeadZone = 0.2f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gamepad Cursor")
    bool bGamepadCursorEnabled = false;

private:
    FVector2D VirtualCursorPosition;
    FVector2D CurrentStickValue;   // 当前摇杆值（-1..1）

    void UpdateMousePosition();
    void HandleCursorMove(float DeltaTime);

    // 轴回调函数
    void AxisLeftStickX(float Value);
    void AxisLeftStickY(float Value);

    APlayerController* GetOwningPlayerController() const;

    // 存储绑定的句柄（用于解绑）
    struct FInputAxisBinding* StickXBinding = nullptr;
    struct FInputAxisBinding* StickYBinding = nullptr;

    void BindAxisInputs();
    void UnbindAxisInputs();
};