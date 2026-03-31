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

    FUIInfo()
        : Layer(EUIPanelLayer::Middle)
        , State(EUIState::Hidden)
        , WidgetInstance(nullptr)
        , bIsPreloaded(false)
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

    FUILayerNode()
        : UIName(NAME_None)
        , Layer(EUIPanelLayer::None)
        , Widget(nullptr)
    {
    }

    FUILayerNode(FName InUIName, EUIPanelLayer InLayer, UUserWidget* InWidget)
        : UIName(InUIName)
        , Layer(InLayer)
        , Widget(InWidget)
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
    UUserWidget* ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer = EUIPanelLayer::Middle, UObject* Data = nullptr, FName UIName = "", EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive);

    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUIByName(FName UIName, UObject* Data = nullptr, EUIOpenPolicy OpenPolicy = EUIOpenPolicy::Additive);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideUI(FName UIName);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseUI(FName UIName, bool bDestroyInstance = true);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseAllUI();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseTopUI();

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

    // 私有方法
    void AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer);
    void RemoveFromStack(FName UIName);
    void UpdateStackOrder();
    void SafeRemoveWidget(UUserWidget* Widget);

    void DeactivatePreviousUIInput();
    void ActivateTopUIInput();
    void HandleStackChange();

    bool RegisterUIFromConfig(const struct FUIConfigData& Config);
    void RegisterAllUIsFromConfig();
    TSubclassOf<UUserWidget> LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr);
    UInputMappingContext* LoadInputMappingContext(const TSoftObjectPtr<UInputMappingContext>& SoftObjectPtr);

    UWidget* FindFirstFocusableWidget(UWidget* RootWidget) const;

    APlayerController* GetPlayerController() const;
    virtual UWorld* GetWorld() const override;
};