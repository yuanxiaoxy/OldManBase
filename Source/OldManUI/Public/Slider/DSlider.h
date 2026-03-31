#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SLeafWidget.h"
#include "DSlider.generated.h"

// 普通委托，用于 Slate 控件内部回调
DECLARE_DELEGATE_OneParam(FOnSliderValueChanged, float);

// 动态多播委托，用于 UMG 暴露给蓝图
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDSliderValueChanged, float, Value);

/**
 * Slate 控件类：处理绘制与鼠标交互
 */
class SDSlider : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SDSlider) {}
        SLATE_ATTRIBUTE(float, Value)
        SLATE_ATTRIBUTE(float, MinValue)
        SLATE_ATTRIBUTE(float, MaxValue)
        SLATE_ATTRIBUTE(EOrientation, Orientation)
        SLATE_ATTRIBUTE(FVector2D, SliderBarSize)
        SLATE_ATTRIBUTE(FVector2D, ThumbSize)
        SLATE_ATTRIBUTE(const FSlateBrush*, LeftTrackBrush)
        SLATE_ATTRIBUTE(const FSlateBrush*, RightTrackBrush)
        SLATE_ATTRIBUTE(const FSlateBrush*, ThumbBrush)
        SLATE_EVENT(FOnSliderValueChanged, OnValueChanged)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    // 属性设置
    void SetValue(float InValue);
    void SetMinValue(float InMin);
    void SetMaxValue(float InMax);
    void SetOrientation(EOrientation InOrientation);
    void SetSliderBarSize(FVector2D InSize);
    void SetThumbSize(FVector2D InSize);
    void SetLeftTrackBrush(const FSlateBrush* InBrush);
    void SetRightTrackBrush(const FSlateBrush* InBrush);
    void SetThumbBrush(const FSlateBrush* InBrush);
    void SetThumbColorNormal(FLinearColor InColor);
    void SetThumbColorHovered(FLinearColor InColor);
    void SetThumbColorDragged(FLinearColor InColor);

    // 刷新绘制
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
        const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& InWidgetStyle,
        bool bParentEnabled) const override;

    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;
    virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

    virtual FVector2D ComputeDesiredSize(float) const override;

private:
    // 从位置计算值
    float PositionToValue(const FGeometry& Geometry, const FVector2D& Position) const;
    // 获取滑块柄的位置（相对于控件局部坐标）
    FVector2D GetThumbPosition(const FGeometry& Geometry) const;

    TAttribute<float> ValueAttribute;
    TAttribute<float> MinValueAttribute;
    TAttribute<float> MaxValueAttribute;
    TAttribute<EOrientation> OrientationAttribute;
    TAttribute<FVector2D> SliderBarSizeAttribute;
    TAttribute<FVector2D> ThumbSizeAttribute;
    TAttribute<const FSlateBrush*> LeftTrackBrushAttribute;
    TAttribute<const FSlateBrush*> RightTrackBrushAttribute;
    TAttribute<const FSlateBrush*> ThumbBrushAttribute;
    FOnSliderValueChanged OnValueChangedDelegate;

    bool bIsDragging;
    bool bIsHovered;
    float CurrentValue;
    FVector2D LastMousePosition;

    FLinearColor ThumbColorNormal;
    FLinearColor ThumbColorHovered;
    FLinearColor ThumbColorDragged;
};

/**
 * UMG 控件类：包装 SDSlider，提供设计时属性
 */
UCLASS()
class UDSlider : public UWidget
{
    GENERATED_BODY()

public:
    UDSlider(const FObjectInitializer& ObjectInitializer);

    // 值范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
    float MinValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
    float MaxValue;

    // 方向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
    TEnumAsByte<EOrientation> Orientation;

    // 轨道整体尺寸（像素）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
    FVector2D SliderBarSize;

    // 滑块柄尺寸（像素）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider")
    FVector2D ThumbSize;

    // 轨道左侧图片（填充部分）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Images")
    FSlateBrush LeftTrackBrush;

    // 轨道右侧图片（背景部分）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Images")
    FSlateBrush RightTrackBrush;

    // 滑块柄图片
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Images")
    FSlateBrush ThumbBrush;

    // 滑块柄颜色配置（普通、悬停、拖动）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Thumb")
    FLinearColor ThumbColorNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Thumb")
    FLinearColor ThumbColorHovered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slider|Thumb")
    FLinearColor ThumbColorDragged;

    // 值变化事件（蓝图可绑定）
    UPROPERTY(BlueprintAssignable, Category = "Slider")
    FOnDSliderValueChanged OnValueChanged;

public:
    // UWidget 接口
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
    virtual const FText GetPaletteCategory() override;
#endif

    // 蓝图调用函数
    UFUNCTION(BlueprintCallable, Category = "Slider")
    float GetValue() const;

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetValue(float InValue);

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetMinValue(float InMin);

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetMaxValue(float InMax);

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetOrientation(EOrientation InOrientation);

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetSliderBarSize(FVector2D InSize);

    UFUNCTION(BlueprintCallable, Category = "Slider")
    void SetThumbSize(FVector2D InSize);

    UFUNCTION(BlueprintCallable, Category = "Slider|Images")
    void SetLeftTrackBrush(const FSlateBrush& InBrush);

    UFUNCTION(BlueprintCallable, Category = "Slider|Images")
    void SetRightTrackBrush(const FSlateBrush& InBrush);

    UFUNCTION(BlueprintCallable, Category = "Slider|Images")
    void SetThumbBrush(const FSlateBrush& InBrush);

    UFUNCTION(BlueprintCallable, Category = "Slider|Thumb")
    void SetThumbColorNormal(FLinearColor InColor);

    UFUNCTION(BlueprintCallable, Category = "Slider|Thumb")
    void SetThumbColorHovered(FLinearColor InColor);

    UFUNCTION(BlueprintCallable, Category = "Slider|Thumb")
    void SetThumbColorDragged(FLinearColor InColor);

protected:
    TSharedPtr<SDSlider> MySlider;

    // 内部回调，将 Slate 的普通委托转发给动态多播委托
    void HandleValueChanged(float NewValue);
};