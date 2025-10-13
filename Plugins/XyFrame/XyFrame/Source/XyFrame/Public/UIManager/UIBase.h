// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/CheckBox.h"
#include "Components/ProgressBar.h"
#include "UITypes.h"
#include "UIBase.generated.h"

// �ؼ���Ϣ
USTRUCT(BlueprintType)
struct FUIControlInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FString ControlPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIControlType ControlType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UWidget* Widget;

    FUIControlInfo()
        : ControlType(EUIControlType::Button)
        , Widget(nullptr)
    {
    }
};

// UI�ؼ��¼�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, const FString&, ControlPath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckBoxChanged, const FString&, ControlPath, bool, IsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSliderValueChanged, const FString&, ControlPath, float, Value);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIBase : public UUserWidget
{
    GENERATED_BODY()

public:
    // ���캯��
    UUIBase(const FObjectInitializer& ObjectInitializer);

    // ========== �������� ==========

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void ShowUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void HideUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void CloseUI();

    // ========== �������� ==========

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void SetData(UObject* Data);

    // ========== �ؼ���ȡ ==========

    // ��ȡ��ť
    UFUNCTION(BlueprintCallable, Category = "UI")
    UButton* GetButton(const FString& ControlPath) const;

    // ��ȡ�ı�
    UFUNCTION(BlueprintCallable, Category = "UI")
    UTextBlock* GetText(const FString& ControlPath) const;

    // ��ȡͼƬ
    UFUNCTION(BlueprintCallable, Category = "UI")
    UImage* GetImage(const FString& ControlPath) const;

    // ��ȡ������
    UFUNCTION(BlueprintCallable, Category = "UI")
    USlider* GetSlider(const FString& ControlPath) const;

    // ��ȡ��ѡ��
    UFUNCTION(BlueprintCallable, Category = "UI")
    UCheckBox* GetCheckBox(const FString& ControlPath) const;

    // ��ȡ������
    UFUNCTION(BlueprintCallable, Category = "UI")
    UProgressBar* GetProgressBar(const FString& ControlPath) const;

    // ========== �ؼ����� ==========

    // �����ı�����
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetText(const FString& ControlPath, const FString& Content);

    // ����ͼƬ
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetImage(const FString& ControlPath, UTexture2D* Texture);

    // ���ý���
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetProgress(const FString& ControlPath, float Progress);

    // ========== �ؼ��¼� ==========

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnButtonClicked OnButtonClicked;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnCheckBoxChanged OnCheckBoxChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnSliderValueChanged OnSliderValueChanged;

protected:
    // �ؼ��ֵ�
    UPROPERTY()
    TMap<FString, FUIControlInfo> ControlDictionary;

    // ��ʼ����־
    bool bIsInitialized;

    // �ؼ���·����ӳ��
    UPROPERTY()
    TMap<UWidget*, FString> WidgetPathMap;

    // ========== �¼����� ==========

    // ���ؼ�·�����¼�������
    UFUNCTION()
    virtual void HandleButtonClick(const FString& ControlPath);

    UFUNCTION()
    virtual void HandleCheckBoxChanged(const FString& ControlPath, bool IsChecked);

    UFUNCTION()
    virtual void HandleSliderValueChanged(const FString& ControlPath, float Value);

    // �ڲ��¼�������
    UFUNCTION()
    void HandleButtonClickInternal();

    UFUNCTION()
    void HandleCheckBoxChangedInternal(bool IsChecked);

    UFUNCTION()
    void HandleSliderValueChangedInternal(float Value);

    // ========== ��ʼ������ ==========

    // ��ʼ���ؼ�
    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void InitializeControls();

    // �����ӿؼ�
    void FindChildControls();

    // ע��ؼ��¼�
    void RegisterControlEvents();

    // ========== �������� ==========

    // ��ȡ�ؼ�·��
    FString GetControlPath(UWidget* Widget) const;

    // ��ӿؼ����ֵ�
    void AddControlToDictionary(const FString& Path, UWidget* Widget, EUIControlType Type);

    // ���ݿؼ�����·��
    FString FindControlPathByWidget(UWidget* Widget) const;

    // ��ȡ��ǰ�¼�Դ�ؼ�
    UWidget* GetCurrentEventSourceWidget() const;

private:
    // �ڲ���ʼ��
    void InternalInitialize();
};