#include "Slider/DSlider.h"
#include "Rendering/DrawElements.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateBrush.h"
#include "Widgets/InvalidateWidgetReason.h"

// ============================= SDSlider 实现 =============================
void SDSlider::Construct(const FArguments& InArgs)
{
    ValueAttribute = InArgs._Value;
    MinValueAttribute = InArgs._MinValue;
    MaxValueAttribute = InArgs._MaxValue;
    OrientationAttribute = InArgs._Orientation;
    SliderBarSizeAttribute = InArgs._SliderBarSize;
    ThumbSizeAttribute = InArgs._ThumbSize;
    LeftTrackBrushAttribute = InArgs._LeftTrackBrush;
    RightTrackBrushAttribute = InArgs._RightTrackBrush;
    ThumbBrushAttribute = InArgs._ThumbBrush;
    OnValueChangedDelegate = InArgs._OnValueChanged;

    bIsDragging = false;
    bIsHovered = false;
    CurrentValue = ValueAttribute.Get();

    // 默认颜色
    ThumbColorNormal = FLinearColor::White;
    ThumbColorHovered = FLinearColor(0.8f, 0.8f, 1.0f);
    ThumbColorDragged = FLinearColor(0.6f, 0.6f, 1.0f);

    SetClipping(EWidgetClipping::ClipToBounds);
}

void SDSlider::SetValue(float InValue)
{
    CurrentValue = FMath::Clamp(InValue, MinValueAttribute.Get(), MaxValueAttribute.Get());
    ValueAttribute.Set(CurrentValue);
    Invalidate(EInvalidateWidgetReason::Paint);
    OnValueChangedDelegate.ExecuteIfBound(CurrentValue);
}

void SDSlider::SetMinValue(float InMin)
{
    MinValueAttribute.Set(InMin);
    CurrentValue = FMath::Clamp(CurrentValue, InMin, MaxValueAttribute.Get());
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetMaxValue(float InMax)
{
    MaxValueAttribute.Set(InMax);
    CurrentValue = FMath::Clamp(CurrentValue, MinValueAttribute.Get(), InMax);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetOrientation(EOrientation InOrientation)
{
    OrientationAttribute.Set(InOrientation);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetSliderBarSize(FVector2D InSize)
{
    SliderBarSizeAttribute.Set(InSize);
    Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
}

void SDSlider::SetThumbSize(FVector2D InSize)
{
    ThumbSizeAttribute.Set(InSize);
    Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Paint);
}

void SDSlider::SetLeftTrackBrush(const FSlateBrush* InBrush)
{
    LeftTrackBrushAttribute.Set(InBrush);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetRightTrackBrush(const FSlateBrush* InBrush)
{
    RightTrackBrushAttribute.Set(InBrush);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetThumbBrush(const FSlateBrush* InBrush)
{
    ThumbBrushAttribute.Set(InBrush);
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetThumbColorNormal(FLinearColor InColor)
{
    ThumbColorNormal = InColor;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetThumbColorHovered(FLinearColor InColor)
{
    ThumbColorHovered = InColor;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::SetThumbColorDragged(FLinearColor InColor)
{
    ThumbColorDragged = InColor;
    Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D SDSlider::ComputeDesiredSize(float) const
{
    return SliderBarSizeAttribute.Get();
}

int32 SDSlider::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    // 获取控件的全局变换
    FSlateLayoutTransform LayoutTransform = AllottedGeometry.GetAccumulatedLayoutTransform();
    FSlateRenderTransform LocalRenderTransform = AllottedGeometry.GetAccumulatedRenderTransform();

    // 1. 绘制右侧轨道（背景）—— 始终拉伸
    const FSlateBrush* RightBrush = RightTrackBrushAttribute.Get();
    if (RightBrush)
    {
        FVector2D BarSize = SliderBarSizeAttribute.Get();
        FPaintGeometry PaintGeom(LayoutTransform, LocalRenderTransform, BarSize, false);
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId,
            PaintGeom,
            RightBrush,
            ESlateDrawEffect::None,
            RightBrush->GetTint(InWidgetStyle)
        );
    }

    // 2. 绘制左侧轨道（填充部分）
    const FSlateBrush* LeftBrush = LeftTrackBrushAttribute.Get();
    if (LeftBrush && CurrentValue > MinValueAttribute.Get())
    {
        float NormValue = (CurrentValue - MinValueAttribute.Get()) / (MaxValueAttribute.Get() - MinValueAttribute.Get());
        FVector2D BarSize = SliderBarSizeAttribute.Get();
        FVector2D FillSize;
        if (OrientationAttribute.Get() == Orient_Horizontal)
        {
            FillSize = FVector2D(BarSize.X * NormValue, BarSize.Y);
        }
        else
        {
            FillSize = FVector2D(BarSize.X, BarSize.Y * NormValue);
        }

        if (bIsDragging)
        {
            // 拖动模式：使用 UVRegion 实现图片原始大小，只显示填充区域内的部分
            FVector2D ImageSize = LeftBrush->ImageSize;
            if (ImageSize.X > 0.0f && ImageSize.Y > 0.0f && FillSize.X > 0.0f && FillSize.Y > 0.0f)
            {
                // 计算 UV 区域（归一化坐标）
                FVector2D UVMax = FillSize / ImageSize;
                FBox2D UVRegion(FVector2D(0.0f, 0.0f), UVMax);
                // 创建临时画刷，复制原始画刷并设置 UVRegion
                FSlateBrush TempBrush = *LeftBrush;
                TempBrush.SetUVRegion(UVRegion);
                // 绘制区域为填充区域大小（因为我们只显示裁剪后的部分）
                FPaintGeometry PaintGeom(LayoutTransform, LocalRenderTransform, FillSize, false);
                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId + 1,
                    PaintGeom,
                    &TempBrush,
                    ESlateDrawEffect::None,
                    TempBrush.GetTint(InWidgetStyle)
                );
            }
            else
            {
                // 图片尺寸无效，回退到拉伸模式
                FPaintGeometry PaintGeom(LayoutTransform, LocalRenderTransform, FillSize, false);
                FSlateDrawElement::MakeBox(
                    OutDrawElements,
                    LayerId + 1,
                    PaintGeom,
                    LeftBrush,
                    ESlateDrawEffect::None,
                    LeftBrush->GetTint(InWidgetStyle)
                );
            }
        }
        else
        {
            // 正常模式：拉伸缩放以适应填充区域
            FPaintGeometry PaintGeom(LayoutTransform, LocalRenderTransform, FillSize, false);
            FSlateDrawElement::MakeBox(
                OutDrawElements,
                LayerId + 1,
                PaintGeom,
                LeftBrush,
                ESlateDrawEffect::None,
                LeftBrush->GetTint(InWidgetStyle)
            );
        }
    }

    // 3. 绘制滑块柄，根据状态选择颜色
    const FSlateBrush* ThumbBrush = ThumbBrushAttribute.Get();
    if (ThumbBrush)
    {
        FVector2D ThumbPos = GetThumbPosition(AllottedGeometry);
        FVector2D ThumbSize = ThumbSizeAttribute.Get();

        // 构造滑块柄的布局变换（添加偏移）
        FSlateLayoutTransform ThumbLayoutTransform = LayoutTransform.Concatenate(FSlateLayoutTransform(ThumbPos));
        FPaintGeometry PaintGeom(ThumbLayoutTransform, LocalRenderTransform, ThumbSize, false);

        FLinearColor TintColor = ThumbColorNormal;
        if (bIsDragging)
            TintColor = ThumbColorDragged;
        else if (bIsHovered)
            TintColor = ThumbColorHovered;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId + 2,
            PaintGeom,
            ThumbBrush,
            ESlateDrawEffect::None,
            TintColor
        );
    }

    return LayerId + 3;
}

FVector2D SDSlider::GetThumbPosition(const FGeometry& Geometry) const
{
    FVector2D BarSize = SliderBarSizeAttribute.Get();
    FVector2D ThumbSize = ThumbSizeAttribute.Get();
    float NormValue = (CurrentValue - MinValueAttribute.Get()) / (MaxValueAttribute.Get() - MinValueAttribute.Get());

    if (OrientationAttribute.Get() == Orient_Horizontal)
    {
        float X = NormValue * (BarSize.X - ThumbSize.X);
        X = FMath::Clamp(X, 0.f, BarSize.X - ThumbSize.X);
        float Y = (BarSize.Y - ThumbSize.Y) * 0.5f;
        return FVector2D(X, Y);
    }
    else
    {
        float Y = NormValue * (BarSize.Y - ThumbSize.Y);
        Y = FMath::Clamp(Y, 0.f, BarSize.Y - ThumbSize.Y);
        float X = (BarSize.X - ThumbSize.X) * 0.5f;
        return FVector2D(X, Y);
    }
}

float SDSlider::PositionToValue(const FGeometry& Geometry, const FVector2D& Position) const
{
    FVector2D LocalPos = Geometry.AbsoluteToLocal(Position);
    FVector2D BarSize = SliderBarSizeAttribute.Get();
    float NormValue;

    if (OrientationAttribute.Get() == Orient_Horizontal)
    {
        NormValue = LocalPos.X / BarSize.X;
    }
    else
    {
        NormValue = LocalPos.Y / BarSize.Y;
    }
    NormValue = FMath::Clamp(NormValue, 0.f, 1.f);
    return MinValueAttribute.Get() + NormValue * (MaxValueAttribute.Get() - MinValueAttribute.Get());
}

FReply SDSlider::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = true;
        LastMousePosition = MouseEvent.GetScreenSpacePosition();
        float NewValue = PositionToValue(MyGeometry, LastMousePosition);
        SetValue(NewValue);
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }
    return FReply::Unhandled();
}

FReply SDSlider::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        bIsDragging = false;
        Invalidate(EInvalidateWidgetReason::Paint);
        return FReply::Handled().ReleaseMouseCapture();
    }
    return FReply::Unhandled();
}

FReply SDSlider::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bIsDragging)
    {
        FVector2D CurrentPos = MouseEvent.GetScreenSpacePosition();
        if (CurrentPos != LastMousePosition)
        {
            float NewValue = PositionToValue(MyGeometry, CurrentPos);
            SetValue(NewValue);
            LastMousePosition = CurrentPos;
        }
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

void SDSlider::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
    bIsDragging = false;
    Invalidate(EInvalidateWidgetReason::Paint);
}

void SDSlider::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    bIsHovered = true;
    Invalidate(EInvalidateWidgetReason::Paint);
    SLeafWidget::OnMouseEnter(MyGeometry, MouseEvent);
}

void SDSlider::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    bIsHovered = false;
    Invalidate(EInvalidateWidgetReason::Paint);
    SLeafWidget::OnMouseLeave(MouseEvent);
}

// ============================= UDSlider 实现 =============================
UDSlider::UDSlider(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , Value(0.5f)
    , MinValue(0.f)
    , MaxValue(1.f)
    , Orientation(Orient_Horizontal)
    , SliderBarSize(200.f, 20.f)
    , ThumbSize(20.f, 20.f)
    , ThumbColorNormal(FLinearColor::White)
    , ThumbColorHovered(FLinearColor(0.8f, 0.8f, 1.0f))
    , ThumbColorDragged(FLinearColor(0.6f, 0.6f, 1.0f))
{
    // 左侧轨道默认使用 Image 模式，以支持 UVRegion
    LeftTrackBrush.DrawAs = ESlateBrushDrawType::Image;
    LeftTrackBrush.TintColor = FLinearColor(0.2f, 0.6f, 1.f);
    // 右侧轨道：可使用 Box 拉伸填充背景
    RightTrackBrush.DrawAs = ESlateBrushDrawType::Box;
    RightTrackBrush.TintColor = FLinearColor(0.3f, 0.3f, 0.3f);
    // 滑块柄：默认使用 Box
    ThumbBrush.DrawAs = ESlateBrushDrawType::Box;
    ThumbBrush.TintColor = FLinearColor(1.f, 1.f, 1.f);
}

void UDSlider::HandleValueChanged(float NewValue)
{
    Value = NewValue;
    OnValueChanged.Broadcast(NewValue);
}

TSharedRef<SWidget> UDSlider::RebuildWidget()
{
    MySlider = SNew(SDSlider)
        .Value(Value)
        .MinValue(MinValue)
        .MaxValue(MaxValue)
        .Orientation(Orientation)
        .SliderBarSize(SliderBarSize)
        .ThumbSize(ThumbSize)
        .LeftTrackBrush(&LeftTrackBrush)
        .RightTrackBrush(&RightTrackBrush)
        .ThumbBrush(&ThumbBrush)
        .OnValueChanged(FOnSliderValueChanged::CreateUObject(this, &UDSlider::HandleValueChanged));

    if (MySlider.IsValid())
    {
        MySlider->SetThumbColorNormal(ThumbColorNormal);
        MySlider->SetThumbColorHovered(ThumbColorHovered);
        MySlider->SetThumbColorDragged(ThumbColorDragged);
    }

    return MySlider.ToSharedRef();
}

void UDSlider::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    if (MySlider.IsValid())
    {
        MySlider->SetValue(Value);
        MySlider->SetMinValue(MinValue);
        MySlider->SetMaxValue(MaxValue);
        MySlider->SetOrientation(Orientation);
        MySlider->SetSliderBarSize(SliderBarSize);
        MySlider->SetThumbSize(ThumbSize);
        MySlider->SetLeftTrackBrush(&LeftTrackBrush);
        MySlider->SetRightTrackBrush(&RightTrackBrush);
        MySlider->SetThumbBrush(&ThumbBrush);
        MySlider->SetThumbColorNormal(ThumbColorNormal);
        MySlider->SetThumbColorHovered(ThumbColorHovered);
        MySlider->SetThumbColorDragged(ThumbColorDragged);
    }
}

void UDSlider::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    MySlider.Reset();
}

#if WITH_EDITOR
const FText UDSlider::GetPaletteCategory()
{
    return NSLOCTEXT("DSlider", "PaletteCategory", "Custom Controls");
}
#endif

float UDSlider::GetValue() const
{
    return Value;
}

void UDSlider::SetValue(float InValue)
{
    Value = FMath::Clamp(InValue, MinValue, MaxValue);
    if (MySlider.IsValid())
    {
        MySlider->SetValue(Value);
    }
}

void UDSlider::SetMinValue(float InMin)
{
    MinValue = InMin;
    Value = FMath::Clamp(Value, MinValue, MaxValue);
    if (MySlider.IsValid())
    {
        MySlider->SetMinValue(MinValue);
        MySlider->SetValue(Value);
    }
}

void UDSlider::SetMaxValue(float InMax)
{
    MaxValue = InMax;
    Value = FMath::Clamp(Value, MinValue, MaxValue);
    if (MySlider.IsValid())
    {
        MySlider->SetMaxValue(MaxValue);
        MySlider->SetValue(Value);
    }
}

void UDSlider::SetOrientation(EOrientation InOrientation)
{
    Orientation = InOrientation;
    if (MySlider.IsValid())
    {
        MySlider->SetOrientation(Orientation);
    }
}

void UDSlider::SetSliderBarSize(FVector2D InSize)
{
    SliderBarSize = InSize;
    if (MySlider.IsValid())
    {
        MySlider->SetSliderBarSize(SliderBarSize);
    }
}

void UDSlider::SetThumbSize(FVector2D InSize)
{
    ThumbSize = InSize;
    if (MySlider.IsValid())
    {
        MySlider->SetThumbSize(ThumbSize);
    }
}

void UDSlider::SetLeftTrackBrush(const FSlateBrush& InBrush)
{
    LeftTrackBrush = InBrush;
    if (MySlider.IsValid())
    {
        MySlider->SetLeftTrackBrush(&LeftTrackBrush);
    }
}

void UDSlider::SetRightTrackBrush(const FSlateBrush& InBrush)
{
    RightTrackBrush = InBrush;
    if (MySlider.IsValid())
    {
        MySlider->SetRightTrackBrush(&RightTrackBrush);
    }
}

void UDSlider::SetThumbBrush(const FSlateBrush& InBrush)
{
    ThumbBrush = InBrush;
    if (MySlider.IsValid())
    {
        MySlider->SetThumbBrush(&ThumbBrush);
    }
}

void UDSlider::SetThumbColorNormal(FLinearColor InColor)
{
    ThumbColorNormal = InColor;
    if (MySlider.IsValid())
    {
        MySlider->SetThumbColorNormal(ThumbColorNormal);
    }
}

void UDSlider::SetThumbColorHovered(FLinearColor InColor)
{
    ThumbColorHovered = InColor;
    if (MySlider.IsValid())
    {
        MySlider->SetThumbColorHovered(ThumbColorHovered);
    }
}

void UDSlider::SetThumbColorDragged(FLinearColor InColor)
{
    ThumbColorDragged = InColor;
    if (MySlider.IsValid())
    {
        MySlider->SetThumbColorDragged(ThumbColorDragged);
    }
}