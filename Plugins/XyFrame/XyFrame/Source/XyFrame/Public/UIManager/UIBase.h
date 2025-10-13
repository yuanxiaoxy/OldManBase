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

// 控件信息
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

// UI控件事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonClicked, const FString&, ControlPath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckBoxChanged, const FString&, ControlPath, bool, IsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSliderValueChanged, const FString&, ControlPath, float, Value);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIBase : public UUserWidget
{
    GENERATED_BODY()

public:
    // 构造函数
    UUIBase(const FObjectInitializer& ObjectInitializer);

    // ========== 生命周期 ==========

    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void ShowUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void HideUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void CloseUI();

    // ========== 数据设置 ==========

    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void SetData(UObject* Data);

    // ========== 控件获取 ==========

    // 获取按钮
    UFUNCTION(BlueprintCallable, Category = "UI")
    UButton* GetButton(const FString& ControlPath) const;

    // 获取文本
    UFUNCTION(BlueprintCallable, Category = "UI")
    UTextBlock* GetText(const FString& ControlPath) const;

    // 获取图片
    UFUNCTION(BlueprintCallable, Category = "UI")
    UImage* GetImage(const FString& ControlPath) const;

    // 获取滑动条
    UFUNCTION(BlueprintCallable, Category = "UI")
    USlider* GetSlider(const FString& ControlPath) const;

    // 获取复选框
    UFUNCTION(BlueprintCallable, Category = "UI")
    UCheckBox* GetCheckBox(const FString& ControlPath) const;

    // 获取进度条
    UFUNCTION(BlueprintCallable, Category = "UI")
    UProgressBar* GetProgressBar(const FString& ControlPath) const;

    // ========== 控件设置 ==========

    // 设置文本内容
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetText(const FString& ControlPath, const FString& Content);

    // 设置图片
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetImage(const FString& ControlPath, UTexture2D* Texture);

    // 设置进度
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetProgress(const FString& ControlPath, float Progress);

    // ========== 控件事件 ==========

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnButtonClicked OnButtonClicked;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnCheckBoxChanged OnCheckBoxChanged;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnSliderValueChanged OnSliderValueChanged;

protected:
    // 控件字典
    UPROPERTY()
    TMap<FString, FUIControlInfo> ControlDictionary;

    // 初始化标志
    bool bIsInitialized;

    // 控件到路径的映射
    UPROPERTY()
    TMap<UWidget*, FString> WidgetPathMap;

    // ========== 事件处理 ==========

    // 带控件路径的事件处理函数
    UFUNCTION()
    virtual void HandleButtonClick(const FString& ControlPath);

    UFUNCTION()
    virtual void HandleCheckBoxChanged(const FString& ControlPath, bool IsChecked);

    UFUNCTION()
    virtual void HandleSliderValueChanged(const FString& ControlPath, float Value);

    // 内部事件处理函数
    UFUNCTION()
    void HandleButtonClickInternal();

    UFUNCTION()
    void HandleCheckBoxChangedInternal(bool IsChecked);

    UFUNCTION()
    void HandleSliderValueChangedInternal(float Value);

    // ========== 初始化方法 ==========

    // 初始化控件
    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void InitializeControls();

    // 查找子控件
    void FindChildControls();

    // 注册控件事件
    void RegisterControlEvents();

    // ========== 辅助方法 ==========

    // 获取控件路径
    FString GetControlPath(UWidget* Widget) const;

    // 添加控件到字典
    void AddControlToDictionary(const FString& Path, UWidget* Widget, EUIControlType Type);

    // 根据控件查找路径
    FString FindControlPathByWidget(UWidget* Widget) const;

    // 获取当前事件源控件
    UWidget* GetCurrentEventSourceWidget() const;

private:
    // 内部初始化
    void InternalInitialize();
};