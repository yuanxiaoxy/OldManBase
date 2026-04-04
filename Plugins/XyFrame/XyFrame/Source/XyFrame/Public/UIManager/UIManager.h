// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
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

    // 新增：UI类型
    EUIPanelType PanelType;

    // 新增：是否修改输入
    bool bModifyInput;

    FUIInfo()
        : Layer(EUIPanelLayer::Middle)
        , State(EUIState::Hidden)
        , WidgetInstance(nullptr)
        , bIsPreloaded(false)
        , PanelType(EUIPanelType::Other)
        , bModifyInput(true)
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

    // 新增：是否修改输入
    bool bModifyInput;

    FUILayerNode()
        : UIName(NAME_None)
        , Layer(EUIPanelLayer::None)
        , Widget(nullptr)
        , bModifyInput(true)
    {
    }

    FUILayerNode(FName InUIName, EUIPanelLayer InLayer, UUserWidget* InWidget, bool bInModifyInput = true)
        : UIName(InUIName)
        , Layer(InLayer)
        , Widget(InWidget)
        , bModifyInput(bInModifyInput)
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

    // ========== UI生命周期方法（基于 UIName） ==========
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer = EUIPanelLayer::Middle, UObject* Data = nullptr, FName UIName = "", EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive);

    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUIByName(FName UIName, UObject* Data = nullptr, EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive);

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

    // 配置数据
    UPROPERTY()
    UUIConfigDataAsset* UIConfigData;

    // 世界上下文
    UPROPERTY()
    UWorld* WorldContext;

    // 当前激活输入的UI（弱指针）
    UPROPERTY()
    TWeakObjectPtr<UUIBase> CurrentInputActiveUI;

    // 关闭标志，用于析构时跳过清理操作
    bool bIsShuttingDown;

    // MainPanel 历史栈（记录被自动隐藏的 MainPanel）
    TArray<FName> MainPanelHistoryStack;

    // 递归保护标志，防止 MainPanel 切换死循环
    bool bIsSwitchingMainPanel;

    // 私有方法
    void AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer, bool bModifyInput);
    void RemoveFromStack(FName UIName);
    void UpdateStackOrder();
    void SafeRemoveWidget(UUserWidget* Widget);

    void DeactivatePreviousUIInput();
    void ActivateTopUIInput();
    void HandleStackChange(bool bOverrideInput = true);

    bool RegisterUIFromConfig(const struct FUIConfigData& Config);
    void RegisterAllUIsFromConfig();
    TSubclassOf<UUserWidget> LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr);
    UInputMappingContext* LoadInputMappingContext(const TSoftObjectPtr<UInputMappingContext>& SoftObjectPtr);

    UWidget* FindFirstFocusableWidget(UWidget* RootWidget) const;

    APlayerController* GetPlayerController() const;
    virtual UWorld* GetWorld() const override;

    // 内部方法：直接操作 UI（不触发 UIBase 的公共方法，避免递归）
    void InternalShowUI(UUIBase* UI, UObject* Data);
    void InternalHideUI(UUIBase* UI);
    void InternalCloseUI(UUIBase* UI);

    // 静默隐藏（用于 MainPanel 自动切换，不触发回调）
    void InternalHideUISilent(UUIBase* UI);

    // 关闭指定类型的所有 UI（用于 Notification）
    void CloseAllUIsOfType(EUIPanelType Type);
};