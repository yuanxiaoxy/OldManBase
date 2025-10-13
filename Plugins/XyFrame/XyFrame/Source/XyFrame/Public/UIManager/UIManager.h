// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UITypes.h"
#include "UIManager.generated.h"

// 前向声明
class UUIConfigDataAsset;
class UUIBase;
struct FUIConfigData;  // 添加这个前向声明

// UI信息结构
USTRUCT(BlueprintType)
struct FUIInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FName UIName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> WidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIPanelLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UUserWidget* WidgetInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bIsPreloaded;

    FUIInfo()
        : Layer(EUIPanelLayer::Middle)
        , State(EUIState::Hidden)
        , WidgetInstance(nullptr)
        , bIsPreloaded(false)
    {
    }
};

// UI事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIShown, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIHidden, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIClosed, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIConfigLoaded, bool, bSuccess);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UUIManager)

public:
    // 初始化UI管理器
    UFUNCTION(BlueprintCallable, Category = "UI")
    void InitializeUIManager(UUIConfigDataAsset* ConfigDataAsset = nullptr);

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI", meta = (DisplayName = "Get UI Manager"))
    static UUIManager* GetUIManager() { return GetInstance(); }

    UUIManager();
    virtual ~UUIManager() override;

    // ========== UI生命周期方法 ==========

    // 显示UI界面
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer = EUIPanelLayer::Middle, UObject* Data = nullptr, FName UIName = "");

    // 根据名称显示UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUIByName(FName UIName, UObject* Data = nullptr);

    // 隐藏UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideUI(FName UIName);

    // 关闭UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseUI(FName UIName);

    // 关闭所有UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseAllUI();

    // ========== 配置方法 ==========

    // 加载UI配置
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void LoadUIConfig(UUIConfigDataAsset* ConfigDataAsset);

    // 重新加载UI配置
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void ReloadUIConfig();

    // 预加载UI
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadUIs(const TArray<FName>& UINames);

    // 预加载所有标记为预加载的UI
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadMarkedUIs();

    // ========== UI信息查询 ==========

    // 获取UI实例
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* GetUI(FName UIName) const;

    // 检查UI是否显示中
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsUIVisible(FName UIName) const;

    // 检查UI是否存在
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool DoesUIExist(FName UIName) const;

    // 获取所有活跃的UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllActiveUIs() const;

    // 获取所有注册的UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllRegisteredUIs() const;

    // ========== 层级方法 ==========

    // 设置UI层级
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetUILayer(FName UIName, EUIPanelLayer NewLayer);

    // 获取UI层级
    UFUNCTION(BlueprintCallable, Category = "UI")
    EUIPanelLayer GetUILayer(FName UIName) const;

    // ========== 动画和效果 ==========

    // 淡入效果
    UFUNCTION(BlueprintCallable, Category = "UI")
    void FadeInUI(FName UIName, float Duration = 0.3f);

    // 淡出效果
    UFUNCTION(BlueprintCallable, Category = "UI")
    void FadeOutUI(FName UIName, float Duration = 0.3f);

    // ========== 调试功能 ==========

    // 打印所有UI信息
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintAllUIs();

    // 打印层级信息
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintLayerInfo();

    // 打印配置信息
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintConfigInfo();

    // ========== 事件 ==========

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIShown OnUIShown;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIHidden OnUIHidden;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIClosed OnUIClosed;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIConfigLoaded OnUIConfigLoaded;

private:
    // UI注册表
    UPROPERTY()
    TMap<FName, FUIInfo> UIRegistry;

    // 层级面板
    UPROPERTY()
    TMap<EUIPanelLayer, UCanvasPanel*> LayerPanels;

    // 配置数据
    UPROPERTY()
    UUIConfigDataAsset* UIConfigData;

    // 世界上下文
    UPROPERTY()
    UWorld* WorldContext;

    // 根画布
    UPROPERTY()
    UCanvasPanel* RootCanvas;

    // 私有方法
    void CreateLayerPanels();
    UCanvasPanel* GetOrCreateLayerPanel(EUIPanelLayer Layer);
    void AddToLayer(UUserWidget* Widget, EUIPanelLayer Layer);
    void RemoveFromLayer(UUserWidget* Widget);

    // 配置管理方法
    bool RegisterUIFromConfig(const FUIConfigData& Config);  // 修复这里，添加返回类型
    void RegisterAllUIsFromConfig();
    TSubclassOf<UUserWidget> LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr);

    // 动画处理
    void HandleFadeAnimation(FName UIName, float TargetAlpha, float Duration);

    // 获取世界
    UWorld* GetWorld() const override;
};