// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "UITypes.h"
#include "UIBase.h"
#include "UIManager.generated.h"

class UUIConfigDataAsset;
class UUIBase;
struct FUIConfigData;
class AXyPlayerControllerBase;

USTRUCT(BlueprintType)
struct FUIInfo
{
    GENERATED_BODY()

    UPROPERTY()
    FName UIName;

    UPROPERTY()
    TSubclassOf<UUserWidget> WidgetClass;

    UPROPERTY()
    EUIPanelLayer Layer;

    UPROPERTY()
    EUIState State;

    UPROPERTY()
    UUserWidget* WidgetInstance;

    UPROPERTY()
    bool bIsPreloaded;

    EUIPanelType PanelType;
    bool bModifyInput;
    int32 Priority;
    bool bClosePreviousMainPanel; // 新增

    FUIInfo()
        : Layer(EUIPanelLayer::Middle)
        , State(EUIState::Hidden)
        , WidgetInstance(nullptr)
        , bIsPreloaded(false)
        , PanelType(EUIPanelType::Other)
        , bModifyInput(true)
        , Priority(0)
        , bClosePreviousMainPanel(true)
    {
    }
};

USTRUCT(BlueprintType)
struct FUILayerNode
{
    GENERATED_BODY()

    UPROPERTY()
    FName UIName;

    UPROPERTY()
    EUIPanelLayer Layer;

    UPROPERTY()
    UUserWidget* Widget;

    bool bModifyInput;
    int32 Priority;
    uint64 OpenOrder;

    FUILayerNode()
        : UIName(NAME_None)
        , Layer(EUIPanelLayer::None)
        , Widget(nullptr)
        , bModifyInput(true)
        , Priority(0)
        , OpenOrder(0)
    {
    }

    FUILayerNode(FName InUIName, EUIPanelLayer InLayer, UUserWidget* InWidget, bool bInModifyInput, int32 InPriority, uint64 InOpenOrder)
        : UIName(InUIName)
        , Layer(InLayer)
        , Widget(InWidget)
        , bModifyInput(bInModifyInput)
        , Priority(InPriority)
        , OpenOrder(InOpenOrder)
    {
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIShown, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIHidden, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIClosed, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIConfigLoaded, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUITopChanged, FName, NewTopUI);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UUIManager)

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void InitializeUIManager(UUIConfigDataAsset* ConfigDataAsset = nullptr);

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI", meta = (DisplayName = "Get UI Manager"))
    static UUIManager* GetUIManager() { return GetInstance(); }

    UUIManager();
    virtual ~UUIManager() override;

    // ========== UI生命周期方法 ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer = EUIPanelLayer::Middle, UObject* Data = nullptr, FName UIName = "", EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive, int32 ClosePreviousMainPanelOverride = -1);
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUIByName(FName UIName, UObject* Data = nullptr, EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive, int32 ClosePreviousMainPanelOverride = -1);
    
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideUI(FName UIName, bool bRestorePreviousMainPanel = false);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseUI(FName UIName, bool bDestroyInstance = true, bool bRestorePreviousMainPanel = false);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseAllUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseTopUI();

    // ========== 基于 UI 对象的管理方法 ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowUIByWidget(UUserWidget* Widget, UObject* Data = nullptr);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideUIByWidget(UUserWidget* Widget, bool bRestorePreviousMainPanel = false);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseUIByWidget(UUserWidget* Widget, bool bDestroyInstance = true, bool bRestorePreviousMainPanel = false);

    UFUNCTION(BlueprintCallable, Category = "UI")
    FName GetUINameByWidget(UUserWidget* Widget) const;

    // ========== 输入管理方法 ==========
    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void UpdateTopUIInput();

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    UUIBase* GetCurrentInputActiveUI() const;

    UFUNCTION(BlueprintCallable, Category = "UI|Input")
    void ForceReleaseAllInputs();

    // ========== 配置方法 ==========
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void LoadUIConfig(UUIConfigDataAsset* ConfigDataAsset);

    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void ReloadUIConfig();

    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadUIs(const TArray<FName>& UINames);

    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadMarkedUIs();

    // ========== UI信息查询 ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* GetUI(FName UIName) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsUIVisible(FName UIName) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool DoesUIExist(FName UIName) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllActiveUIs() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllRegisteredUIs() const;

    // ========== 层级栈方法 ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetUILayer(FName UIName, EUIPanelLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "UI")
    EUIPanelLayer GetUILayer(FName UIName) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    FName GetTopUIName() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* GetTopUI() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    int32 GetUIStackDepth() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ClearUIStack();

    // ========== 调试功能 ==========
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintAllUIs();

    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintStackInfo();

    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintConfigInfo();

    // ========== 语言切换支持 ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    void OnLanguageChanged(ELanguageType NewLanguage);

    void RefreshAllActiveUIs();

    // ========== 事件 ==========
    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIShown OnUIShown;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIHidden OnUIHidden;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIClosed OnUIClosed;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIConfigLoaded OnUIConfigLoaded;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUITopChanged OnUITopChanged;

private:
    // UI注册表
    UPROPERTY()
    TMap<FName, FUIInfo> UIRegistry;

    // Widget 到 UIName 的反向映射
    UPROPERTY()
    TMap<UUserWidget*, FName> WidgetToUINameMap;

    // UI层级栈
    UPROPERTY()
    TArray<FUILayerNode> UIStack;

    // MainPanel 历史栈
    TArray<FName> MainPanelHistoryStack;

    // 配置数据
    UPROPERTY()
    UUIConfigDataAsset* UIConfigData;

    // 世界上下文
    UPROPERTY()
    UWorld* WorldContext;

    // 当前激活输入的UI（弱指针）
    UPROPERTY()
    TWeakObjectPtr<UUIBase> CurrentInputActiveUI;

    // 关闭标志
    bool bIsShuttingDown;

    // 输入状态缓存
    EUIInputMode CurrentAppliedInputMode;
    TWeakObjectPtr<UUserWidget> CurrentAppliedFocusWidget;
    bool bCurrentAppliedShowMouse;

    // 打开序号计数器
    uint64 NextOpenOrder;

    // 私有核心方法
    void InternalShowUI(UUIBase* UI, UObject* Data);
    void InternalHideUI(UUIBase* UI);
    void InternalCloseUI(UUIBase* UI);
    void InternalHideUISilent(UUIBase* UI);

    void HandleMainPanelShow(FName NewMainPanelName);
    void HandleMainPanelHide(FName HiddenMainPanelName, bool bPushToHistory);
    bool TryRestorePreviousMainPanel();

    void AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer, bool bModifyInput, int32 Priority);
    void RemoveFromStack(FName UIName);
    void UpdateStackOrder();
    void RefreshAllZOrder();    // 根据逻辑栈顺序，刷新所有活跃UI的ZOrder
    void SafeRemoveWidget(UUserWidget* Widget);

    void DeactivatePreviousUIInput();
    void ActivateTopUIInput();
    void HandleStackChange();

    void RegisterUIFromConfig(const struct FUIConfigData& Config);
    void RegisterAllUIsFromConfig();
    TSubclassOf<UUserWidget> LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr);
    UInputMappingContext* LoadInputMappingContext(const TSoftObjectPtr<UInputMappingContext>& SoftObjectPtr);
    UUserWidget* FindFirstFocusableWidget(UWidget* RootWidget) const;
    APlayerController* GetPlayerController() const;
    virtual UWorld* GetWorld() const override;

    void CloseAllUIsOfType(EUIPanelType Type);

    friend class UUIBase;
};