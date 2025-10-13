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

    // ����
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
    // ��ȡ�����ӿؼ�
    if (!WidgetTree) return;

    TArray<UWidget*> AllWidgets;
    WidgetTree->GetAllWidgets(AllWidgets);

    for (UWidget* Widget : AllWidgets)
    {
        if (!Widget) continue;

        FString ControlPath = GetControlPath(Widget);
        EUIControlType ControlType = EUIControlType::Button; // Ĭ������

        // �жϿؼ�����
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
            continue; // ��֧�ֵ�����
        }

        AddControlToDictionary(ControlPath, Widget, ControlType);
        // ͬʱ��ӵ��ؼ���·����ӳ��
        WidgetPathMap.Add(Widget, ControlPath);
    }

    UE_LOG(LogTemp, Log, TEXT("UUIBase::FindChildControls - Found %d controls"), ControlDictionary.Num());
}

void UUIBase::RegisterControlEvents()
{
    // Ϊÿ���ؼ����¼�
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;

        if (UButton* Button = Cast<UButton>(ControlInfo.Widget))
        {
            // ʹ�ñ�׼�� AddDynamic ��
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

// ��ȡ��ǰ�¼�Դ�Ŀؼ�
UWidget* UUIBase::GetCurrentEventSourceWidget() const
{
    // ʹ�ø��򵥵ķ�����ͨ�������ͣ״̬��ȷ����ǰ�����Ŀؼ�
    // ����һ���򻯵�ʵ�֣���ͨ���ܹ���

    // ������а�ť����ͣ״̬
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
    // ���������д�˷�������������
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

// ========== �¼������� ==========

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

// ========== �ڲ��¼������� ==========

void UUIBase::HandleButtonClickInternal()
{
    // ���Ի�ȡ��ǰ�¼�Դ�Ŀؼ�
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleButtonClick(ControlPath);
        return;
    }

    // ����޷�ͨ���¼�Դȷ����ʹ�ø���ȷ�Ļ��˷���
    // �������а�ť������ĸ���ť���������
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::Button && ControlInfo.Widget)
        {
            // ���������Ӹ���ȷ���ж��߼�
            // �����鰴ť��״̬���Ƿ�ɼ����Ƿ�ɵ����
            if (UButton* Button = Cast<UButton>(ControlInfo.Widget))
            {
                // �򵥵��жϣ������ť�ɼ��ҿɵ��������Ϊ���������¼�Դ
                if (Button->GetIsEnabled() && Button->GetVisibility() == ESlateVisibility::Visible)
                {
                    // �����ͣ״̬��Ϊ����ȷ���ж�
                    if (Button->IsHovered())
                    {
                        HandleButtonClick(ControlInfo.ControlPath);
                        return;
                    }
                }
            }
        }
    }

    // ������з�����ʧ�ܣ�ʹ��δ֪·��
    UE_LOG(LogTemp, Warning, TEXT("UUIBase::HandleButtonClickInternal - Could not determine control path"));
    HandleButtonClick(FString("Unknown"));
}

void UUIBase::HandleCheckBoxChangedInternal(bool IsChecked)
{
    // ���Ի�ȡ��ǰ�¼�Դ�Ŀؼ�
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleCheckBoxChanged(ControlPath, IsChecked);
        return;
    }

    // ���˷������������и�ѡ��
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::CheckBox && ControlInfo.Widget)
        {
            if (UCheckBox* CheckBox = Cast<UCheckBox>(ControlInfo.Widget))
            {
                if (CheckBox->GetIsEnabled() && CheckBox->GetVisibility() == ESlateVisibility::Visible)
                {
                    // �����ͣ״̬
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
    // ���Ի�ȡ��ǰ�¼�Դ�Ŀؼ�
    UWidget* EventSource = GetCurrentEventSourceWidget();
    if (EventSource && WidgetPathMap.Contains(EventSource))
    {
        FString ControlPath = WidgetPathMap[EventSource];
        HandleSliderValueChanged(ControlPath, Value);
        return;
    }

    // ���˷������������л�����
    for (auto& Pair : ControlDictionary)
    {
        FUIControlInfo& ControlInfo = Pair.Value;
        if (ControlInfo.ControlType == EUIControlType::Slider && ControlInfo.Widget)
        {
            if (USlider* Slider = Cast<USlider>(ControlInfo.Widget))
            {
                if (Slider->GetIsEnabled() && Slider->GetVisibility() == ESlateVisibility::Visible)
                {
                    // �����ͣ״̬
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