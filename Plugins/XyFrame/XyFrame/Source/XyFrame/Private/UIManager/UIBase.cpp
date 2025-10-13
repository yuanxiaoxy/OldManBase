// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ProgressBar.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"

UUIBase::UUIBase(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsInitialized = false;
}

void UUIBase::NativeConstruct()
{
    Super::NativeConstruct();

    InternalInitialize();
}

void UUIBase::NativeDestruct()
{
    Super::NativeDestruct();

    // 清理
    ControlDictionary.Empty();
    WidgetPathMap.Empty();
}

void UUIBase::InternalInitialize()
{
    if (bIsInitialized) return;

    InitializeControls();
    bIsInitialized = true;
}

void UUIBase::InitializeControls()
{
    FindChildControls();
    RegisterControlEvents();
}

void UUIBase::FindChildControls()
{
    // 获取所有子控件
    if (!WidgetTree) return;

    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);

    for (UWidget* Widget : AllWidgets)
    {
        if (!Widget) continue;

        FString ControlPath = GetControlPath(Widget);
        EUIControlType ControlType = EUIControlType::Button; // 默认类型

        // 判断控件类型
        if (Cast<UButton>(Widget))
        {
            ControlType = EUIControlType::Button;
        }
        else if (Cast<UTextBlock>(Widget))
        {
            ControlType = EUIControlType::Text;
        }
        else if (Cast<UImage>(Widget))
        {
            ControlType = EUIControlType::Image;
        }
        else if (Cast<USlider>(Widget))
        {
            ControlType = EUIControlType::Slider;
        }
        else if (Cast<UCheckBox>(Widget))
        {
            ControlType = EUIControlType::CheckBox;
        }
        else if (Cast<UProgressBar>(Widget))
        {
            ControlType = EUIControlType::ProgressBar;
        }
        else
        {
            continue; // 不支持的类型
        }

        AddControlToDictionary(ControlPath, Widget, ControlType);
        // 同时添加到控件到路径的映射
        WidgetPathMap.Add(Widget, ControlPath);
    }

    UE_LOG(LogTemp, Log, TEXT("UUIBase::FindChildControls - Found %d controls"), ControlDictionary.Num());
}

void UUIBase::RegisterControlEvents()
{
    // 为每个控件绑定事件
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;

        if (UButton* Button = Cast<UButton>(ControlInfo.Widget))
        {
            // 使用标准的 AddDynamic 绑定
            Button->OnClicked.AddDynamic(this, &UUIBase::HandleButtonClickInternal);
        }
        else if (UCheckBox* CheckBox = Cast<UCheckBox>(ControlInfo.Widget))
        {
            CheckBox->OnCheckStateChanged.AddDynamic(this, &UUIBase::HandleCheckBoxChangedInternal);
        }
        else if (USlider* Slider = Cast<USlider>(ControlInfo.Widget))
        {
            Slider->OnValueChanged.AddDynamic(this, &UUIBase::HandleSliderValueChangedInternal);
        }
    }
}

// 获取当前事件源的控件
UWidget* UUIBase::GetCurrentEventSourceWidget() const
{
    // 使用更简单的方法：通过检查悬停状态来确定当前交互的控件
    // 这是一个简化的实现，但通常能工作

    // 检查所有按钮的悬停状态
    for (auto& Pair : WidgetPathMap)
    {
        UWidget* Widget = Pair.Key;
        if (Widget && Widget->IsHovered())
        {
            return Widget;
        }
    }

    return nullptr;
}

FString UUIBase::GetControlPath(UWidget* Widget) const
{
    if (!Widget) return FString();

    TArray<FString> PathParts;
    UWidget* CurrentWidget = Widget;

    while (CurrentWidget && CurrentWidget != this)
    {
        PathParts.Insert(CurrentWidget->GetName(), 0);
        CurrentWidget = CurrentWidget->GetParent();
    }

    return FString::Join(PathParts, TEXT("/"));
}

void UUIBase::AddControlToDictionary(const FString& Path, UWidget* Widget, EUIControlType Type)
{
    FUIControlInfo ControlInfo;
    ControlInfo.ControlPath = Path;
    ControlInfo.ControlType = Type;
    ControlInfo.Widget = Widget;

    ControlDictionary.Add(Path, ControlInfo);
}

FString UUIBase::FindControlPathByWidget(UWidget* Widget) const
{
    for (const auto& Pair : ControlDictionary)
    {
        if (Pair.Value.Widget == Widget)
        {
            return Pair.Key;
        }
    }
    return FString();
}

void UUIBase::ShowUI()
{
    SetVisibility(ESlateVisibility::Visible);
}

void UUIBase::HideUI()
{
    SetVisibility(ESlateVisibility::Hidden);
}

void UUIBase::CloseUI()
{
    RemoveFromParent();
}

void UUIBase::SetData(UObject* Data)
{
    // 子类可以重写此方法来处理数据
}

UButton* UUIBase::GetButton(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Button)
        {
            return Cast<UButton>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

UTextBlock* UUIBase::GetText(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Text)
        {
            return Cast<UTextBlock>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

UImage* UUIBase::GetImage(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Image)
        {
            return Cast<UImage>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

USlider* UUIBase::GetSlider(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::Slider)
        {
            return Cast<USlider>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

UCheckBox* UUIBase::GetCheckBox(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::CheckBox)
        {
            return Cast<UCheckBox>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

UProgressBar* UUIBase::GetProgressBar(const FString& ControlPath) const
{
    if (const FUIControlInfo* ControlInfo = ControlDictionary.Find(ControlPath))
    {
        if (ControlInfo->ControlType == EUIControlType::ProgressBar)
        {
            return Cast<UProgressBar>(ControlInfo->Widget);
        }
    }
    return nullptr;
}

void UUIBase::SetText(const FString& ControlPath, const FString& Content)
{
    if (UTextBlock* TextBlock = GetText(ControlPath))
    {
        TextBlock->SetText(FText::FromString(Content));
    }
}

void UUIBase::SetImage(const FString& ControlPath, UTexture2D* Texture)
{
    if (UImage* Image = GetImage(ControlPath))
    {
        Image->SetBrushFromTexture(Texture);
    }
}

void UUIBase::SetProgress(const FString& ControlPath, float Progress)
{
    if (UProgressBar* ProgressBar = GetProgressBar(ControlPath))
    {
        ProgressBar->SetPercent(Progress);
    }
}

// ========== 事件处理函数 ==========

void UUIBase::HandleButtonClick(const FString& ControlPath)
{
    OnButtonClicked.Broadcast(ControlPath);
    UE_LOG(LogTemp, Log, TEXT("UUIBase::HandleButtonClick - Button clicked: %s"), *ControlPath);
}

void UUIBase::HandleCheckBoxChanged(const FString& ControlPath, bool IsChecked)
{
    OnCheckBoxChanged.Broadcast(ControlPath, IsChecked);
    UE_LOG(LogTemp, Log, TEXT("UUIBase::HandleCheckBoxChanged - CheckBox changed: %s, Checked: %s"),
        *ControlPath, IsChecked ? TEXT("True") : TEXT("False"));
}

void UUIBase::HandleSliderValueChanged(const FString& ControlPath, float Value)
{
    OnSliderValueChanged.Broadcast(ControlPath, Value);
    UE_LOG(LogTemp, Log, TEXT("UUIBase::HandleSliderValueChanged - Slider changed: %s, Value: %.2f"),
        *ControlPath, Value);
}

// ========== 内部事件处理函数 ==========

void UUIBase::HandleButtonClickInternal()
{
    // 尝试获取当前事件源的控件
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleButtonClick(ControlPath);
        return;
    }

    // 如果无法通过事件源确定，使用更精确的回退方法
    // 遍历所有按钮，检查哪个按钮最近被交互
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::Button && ControlInfo.Widget)
        {
            // 这里可以添加更精确的判断逻辑
            // 例如检查按钮的状态、是否可见、是否可点击等
            if (UButton* Button = Cast<UButton>(ControlInfo.Widget))
            {
                // 简单的判断：如果按钮可见且可点击，就认为它可能是事件源
                if (Button->GetIsEnabled() && Button->GetVisibility() == ESlateVisibility::Visible)
                {
                    // 检查悬停状态作为更精确的判断
                    if (Button->IsHovered())
                    {
                        HandleButtonClick(ControlInfo.ControlPath);
                        return;
                    }
                }
            }
        }
    }

    // 如果所有方法都失败，使用未知路径
    UE_LOG(LogTemp, Warning, TEXT("UUIBase::HandleButtonClickInternal - Could not determine control path"));
    HandleButtonClick(FString("Unknown"));
}

void UUIBase::HandleCheckBoxChangedInternal(bool IsChecked)
{
    // 尝试获取当前事件源的控件
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleCheckBoxChanged(ControlPath, IsChecked);
        return;
    }

    // 回退方法：遍历所有复选框
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::CheckBox && ControlInfo.Widget)
        {
            if (UCheckBox* CheckBox = Cast<UCheckBox>(ControlInfo.Widget))
            {
                if (CheckBox->GetIsEnabled() && CheckBox->GetVisibility() == ESlateVisibility::Visible)
                {
                    // 检查悬停状态
                    if (CheckBox->IsHovered())
                    {
                        HandleCheckBoxChanged(ControlInfo.ControlPath, IsChecked);
                        return;
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UUIBase::HandleCheckBoxChangedInternal - Could not determine control path"));
    HandleCheckBoxChanged(FString("Unknown"), IsChecked);
}

void UUIBase::HandleSliderValueChangedInternal(float Value)
{
    // 尝试获取当前事件源的控件
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleSliderValueChanged(ControlPath, Value);
        return;
    }

    // 回退方法：遍历所有滑动条
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::Slider && ControlInfo.Widget)
        {
            if (USlider* Slider = Cast<USlider>(ControlInfo.Widget))
            {
                if (Slider->GetIsEnabled() && Slider->GetVisibility() == ESlateVisibility::Visible)
                {
                    // 检查悬停状态
                    if (Slider->IsHovered())
                    {
                        HandleSliderValueChanged(ControlInfo.ControlPath, Value);
                        return;
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UUIBase::HandleSliderValueChangedInternal - Could not determine control path"));
    HandleSliderValueChanged(FString("Unknown"), Value);
}