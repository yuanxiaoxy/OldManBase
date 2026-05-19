// UIManager.cpp (完整修改后文件)
// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIManager.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UIManager/UIConfigDataAsset.h"
#include "UIManager/UIBase.h"
#include "LanguageManager/xyLanguageManager.h"
#include "XyCharacter/XyPlayerControllerBase.h"

template<>
UUIManager* TSingleton<UUIManager>::SingletonInstance = nullptr;

UUIManager::UUIManager()
    : bIsShuttingDown(false)
    , CurrentAppliedInputMode(EUIInputMode::GameOnly)
    , CurrentAppliedFocusWidget(nullptr)
    , bCurrentAppliedShowMouse(false)
    , NextOpenOrder(0)
{
    WorldContext = nullptr;
    UIConfigData = nullptr;
    CurrentInputActiveUI = nullptr;
}

UUIManager::~UUIManager()
{
    bIsShuttingDown = true;
    UIStack.Empty();
    UIRegistry.Empty();
    WidgetToUINameMap.Empty();
    MainPanelHistoryStack.Empty();
    CurrentInputActiveUI = nullptr;
}

void UUIManager::InitializeUIManager(UUIConfigDataAsset* ConfigDataAsset)
{
    if (WorldContext == nullptr)
    {
        WorldContext = GetWorld();
    }
    ClearUIStack();
    MainPanelHistoryStack.Empty();
    if (ConfigDataAsset)
    {
        LoadUIConfig(ConfigDataAsset);
    }
}

void UUIManager::InitializeSingleton()
{
    // 可选初始化
}

void UUIManager::LoadUIConfig(UUIConfigDataAsset* ConfigDataAsset)
{
    if (!ConfigDataAsset)
    {
        OnUIConfigLoaded.Broadcast(false);
        return;
    }
    UIConfigData = ConfigDataAsset;
    RegisterAllUIsFromConfig();
    OnUIConfigLoaded.Broadcast(true);

    if (UxyLanguageManager* LangMgr = UxyLanguageManager::GetLanguageManager())
    {
        LangMgr->OnLanguageChanged.AddDynamic(this, &UUIManager::OnLanguageChanged);
    }
}

void UUIManager::ReloadUIConfig()
{
    if (UIConfigData)
    {
        CloseAllUI();
        UIRegistry.Empty();
        WidgetToUINameMap.Empty();
        MainPanelHistoryStack.Empty();
        RegisterAllUIsFromConfig();
    }
}

void UUIManager::RegisterAllUIsFromConfig()
{
    if (!UIConfigData) return;
    TArray<FUIConfigData> AllConfigs = UIConfigData->GetAllUIConfigs();
    for (const FUIConfigData& Config : AllConfigs)
        RegisterUIFromConfig(Config);
    PreloadMarkedUIs();
}

void UUIManager::RegisterUIFromConfig(const FUIConfigData& Config)
{
    if (Config.UIName.IsNone() || Config.WidgetClass.IsNull()) return;
    if (UIRegistry.Contains(Config.UIName)) return;

    FUIInfo UIInfo;
    UIInfo.UIName = Config.UIName;
    UIInfo.WidgetClass = nullptr;
    UIInfo.Layer = Config.DefaultLayer;
    UIInfo.State = EUIState::Hidden;
    UIInfo.WidgetInstance = nullptr;
    UIInfo.bIsPreloaded = Config.bPreload;
    UIInfo.PanelType = Config.PanelType;
    UIInfo.bModifyInput = Config.bModifyInput;
    UIInfo.Priority = Config.Priority;
    UIInfo.bClosePreviousMainPanel = Config.bClosePreviousMainPanel; // 新增
    UIRegistry.Add(Config.UIName, UIInfo);
}

TSubclassOf<UUserWidget> UUIManager::LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr)
{
    return SoftClassPtr.IsNull() ? nullptr : SoftClassPtr.LoadSynchronous();
}

UInputMappingContext* UUIManager::LoadInputMappingContext(const TSoftObjectPtr<UInputMappingContext>& SoftObjectPtr)
{
    if (SoftObjectPtr.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadInputMappingContext - SoftObjectPtr is null"));
        return nullptr;
    }

    FSoftObjectPath ObjectPath = SoftObjectPtr.ToSoftObjectPath();
    FString AssetPath = ObjectPath.ToString();
    UE_LOG(LogTemp, Warning, TEXT("LoadInputMappingContext - Attempting to load: %s"), *AssetPath);

    // 检查资产是否存在于内存中
    if (ObjectPath.ResolveObject())
    {
        UE_LOG(LogTemp, Log, TEXT("LoadInputMappingContext - Asset already in memory: %s"), *AssetPath);
    }

    // 同步加载
    UInputMappingContext* IMC = SoftObjectPtr.LoadSynchronous();
    if (IMC)
    {
        UE_LOG(LogTemp, Log, TEXT("LoadInputMappingContext - Successfully loaded: %s"), *AssetPath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("LoadInputMappingContext - FAILED to load: %s"), *AssetPath);
    }
    return IMC;
}

void UUIManager::PreloadUIs(const TArray<FName>& UINames)
{
    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController) return;

    for (const FName& UIName : UINames)
    {
        if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
        {
            if (!UIInfo->WidgetInstance && UIInfo->WidgetClass)
            {
                TSubclassOf<UUserWidget> WidgetClass = UIInfo->WidgetClass;
                if (!WidgetClass) continue;

                UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
                if (Widget)
                {
                    UIInfo->WidgetInstance = Widget;
                    UIInfo->bIsPreloaded = true;
                    WidgetToUINameMap.Add(Widget, UIName);

                    // 预加载的Widget不需要添加到视口，也不需要设置ZOrder
                    // 之后真正显示时会重新设置ZOrder并添加到视口
                }
            }
        }
    }
}

void UUIManager::PreloadMarkedUIs()
{
    if (!UIConfigData) return;
    TArray<FUIConfigData> PreloadConfigs = UIConfigData->GetPreloadUIConfigs();
    for (const FUIConfigData& Config : PreloadConfigs)
    {
        PreloadUIs({ Config.UIName });
    }
}

// ========== 核心UI生命周期 ==========

UUserWidget* UUIManager::ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer, UObject* Data, FName UIName, EUIOpenPolicy OpenPolicy, int32 ClosePreviousMainPanelOverride)
{
    APlayerController* PlayerController = GetPlayerController();
    if (!PlayerController || !WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::ShowUI - Invalid PlayerController or WidgetClass"));
        return nullptr;
    }

    if (UIName.IsNone())
    {
        UIName = FName(*WidgetClass->GetName().Replace(TEXT("_C"), TEXT("")));
    }

    // 获取配置
    FUIConfigData Config;
    bool bHasConfig = UIConfigData && UIConfigData->GetUIConfig(UIName, Config);
    EUIPanelType PanelType = bHasConfig ? Config.PanelType : EUIPanelType::Other;
    bool bModifyInput = bHasConfig ? Config.bModifyInput : true;
    int32 Priority = bHasConfig ? Config.Priority : 0;
    bool bClosePrevious = bHasConfig ? Config.bClosePreviousMainPanel : true;
    // 参数覆写
    if (ClosePreviousMainPanelOverride == 0) bClosePrevious = false;
    else if (ClosePreviousMainPanelOverride == 1) bClosePrevious = true;

    // 获取实际使用的Widget类（根据语言）
    TSubclassOf<UUserWidget> ActualWidgetClass = bHasConfig ? Config.GetLocalizedWidgetClass() : WidgetClass;
    if (!ActualWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("ShowUI: Failed to get widget class for %s"), *UIName.ToString());
        return nullptr;
    }

    // Notification 类型：自动关闭已存在的同类型 UI
    if (PanelType == EUIPanelType::Notification)
    {
        CloseAllUIsOfType(EUIPanelType::Notification);
    }

    // 打开策略处理
    if (OpenPolicy == EUIOpenPolicy::Replace)
    {
        if (UIStack.Num() > 0)
            CloseUI(UIStack.Last().UIName, true, false);
    }
    else if (OpenPolicy == EUIOpenPolicy::Exclusive)
    {
        TArray<FName> AllUIs = GetAllActiveUIs();
        for (FName Name : AllUIs)
        {
            if (Name != UIName)
                CloseUI(Name, true, false);
        }
    }

    // 检查是否已存在实例
    FUIInfo* UIInfo = UIRegistry.Find(UIName);
    UUserWidget* ExistingWidget = UIInfo ? UIInfo->WidgetInstance : nullptr;
    UUIBase* ExistingUIBase = ExistingWidget ? Cast<UUIBase>(ExistingWidget) : nullptr;

    // 如果已存在且可见，则仅刷新数据和栈顺序
    if (ExistingWidget && UIInfo->State == EUIState::Visible)
    {
        RemoveFromStack(UIName);
        AddToStack(ExistingWidget, UIName, Layer, bModifyInput, Priority);
        HandleStackChange();

        if (ExistingUIBase && Data)
            ExistingUIBase->SetData(Data);

        OnUIShown.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return ExistingWidget;
    }

    // 处理 MainPanel 切换（根据配置决定是否关闭之前的 MainPanel）
    if (PanelType == EUIPanelType::MainPanel && bClosePrevious)
    {
        HandleMainPanelShow(UIName);
    }

    // 如果已存在但未显示（Hidden），则直接显示
    if (ExistingWidget && UIInfo->State == EUIState::Hidden)
    {
        UIInfo->Priority = Priority;
        UIInfo->bClosePreviousMainPanel = bClosePrevious; // 更新配置
        if (ExistingUIBase)
        {
            // 更新配置
            if (bHasConfig)
            {
                ExistingUIBase->SetInputMode(Config.DefaultInputMode);
                ExistingUIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
                ExistingUIBase->PanelType = Config.PanelType;
                ExistingUIBase->bModifyInput = Config.bModifyInput;
                // 更新 IMC 等
            }
            // 关键：使用 InternalShowUI 仅显示，不会触发关闭事件
            InternalShowUI(ExistingUIBase, Data);
        }
        else
        {
            ExistingWidget->SetVisibility(ESlateVisibility::Visible);
        }
        UIInfo->State = EUIState::Visible;
        AddToStack(ExistingWidget, UIName, Layer, bModifyInput, Priority);
        HandleStackChange();  // 此处应只处理输入模式，不应关闭 UI
        OnUIShown.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return ExistingWidget;
    }

    // 创建新UI
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, ActualWidgetClass);
    if (!NewWidget) return nullptr;
    UUIBase* NewUIBase = Cast<UUIBase>(NewWidget);

    // 注册或更新注册表
    if (!UIInfo)
    {
        FUIInfo NewInfo;
        NewInfo.UIName = UIName;
        NewInfo.WidgetClass = ActualWidgetClass;
        NewInfo.Layer = Layer;
        NewInfo.State = EUIState::Visible;
        NewInfo.WidgetInstance = NewWidget;
        NewInfo.bIsPreloaded = false;
        NewInfo.PanelType = PanelType;
        NewInfo.bModifyInput = bModifyInput;
        NewInfo.Priority = Priority;
        NewInfo.bClosePreviousMainPanel = bClosePrevious;
        UIRegistry.Add(UIName, NewInfo);
    }
    else
    {
        UIInfo->WidgetClass = ActualWidgetClass;
        UIInfo->WidgetInstance = NewWidget;
        UIInfo->State = EUIState::Visible;
        UIInfo->Layer = Layer;
        UIInfo->PanelType = PanelType;
        UIInfo->bModifyInput = bModifyInput;
        UIInfo->Priority = Priority;
        UIInfo->bClosePreviousMainPanel = bClosePrevious;
    }
    WidgetToUINameMap.Add(NewWidget, UIName);

    // 配置UIBase
    if (NewUIBase)
    {
        if (bHasConfig)
        {
            NewUIBase->SetInputMode(Config.DefaultInputMode);
            NewUIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
            NewUIBase->PanelType = Config.PanelType;
            NewUIBase->bModifyInput = Config.bModifyInput;
            if (Config.DefaultInputMappingContext)
            {
                UInputMappingContext* IMC = LoadInputMappingContext(Config.DefaultInputMappingContext);
                if (IMC)
                    NewUIBase->SetInputMappingContext(IMC, Config.InputPriority);
            }
        }
    }

    // 添加到视口（设置初始ZOrder）
    if (!NewWidget->IsInViewport())
        NewWidget->AddToViewport(0);

    InternalShowUI(NewUIBase, Data);

    AddToStack(NewWidget, UIName, Layer, bModifyInput, Priority);
    HandleStackChange();

    OnUIShown.Broadcast(UIName);
    OnUITopChanged.Broadcast(GetTopUIName());
    return NewWidget;
}

UUserWidget* UUIManager::ShowUIByName(FName UIName, UObject* Data, EUIOpenPolicy OpenPolicy, int32 ClosePreviousMainPanelOverride)
{
    if (UIName.IsNone()) return nullptr;
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        FUIConfigData Config;
        if (UIConfigData && UIConfigData->GetUIConfig(UIName, Config))
        {
            TSubclassOf<UUserWidget> ActualClass = Config.GetLocalizedWidgetClass();
            return ShowUI(ActualClass, UIInfo->Layer, Data, UIName, OpenPolicy, ClosePreviousMainPanelOverride);
        }
        else if (UIInfo->WidgetClass)
        {
            return ShowUI(UIInfo->WidgetClass, UIInfo->Layer, Data, UIName, OpenPolicy, ClosePreviousMainPanelOverride);
        }
    }
    return nullptr;
}

void UUIManager::HideUI(FName UIName, bool bRestorePreviousMainPanel)
{
    if (bIsShuttingDown) return;
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            HideUIByWidget(UIInfo->WidgetInstance, bRestorePreviousMainPanel);
        }
    }
}

void UUIManager::HideUIByWidget(UUserWidget* Widget, bool bRestorePreviousMainPanel)
{
    if (!Widget || bIsShuttingDown) return;
    FName UIName = GetUINameByWidget(Widget);
    if (UIName.IsNone()) return;

    FUIInfo* UIInfo = UIRegistry.Find(UIName);
    if (!UIInfo || UIInfo->State != EUIState::Visible) return;

    bool bIsMainPanel = (UIInfo->PanelType == EUIPanelType::MainPanel);

    if (bIsMainPanel && bRestorePreviousMainPanel)
    {
        if (UUIBase* UIBase = Cast<UUIBase>(Widget))
            InternalHideUI(UIBase);
        else
            Widget->SetVisibility(ESlateVisibility::Hidden);

        UIInfo->State = EUIState::Hidden;
        RemoveFromStack(UIName);
        TryRestorePreviousMainPanel();
        HandleStackChange();
        OnUIHidden.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return;
    }

    if (UUIBase* UIBase = Cast<UUIBase>(Widget))
        InternalHideUI(UIBase);
    else
        Widget->SetVisibility(ESlateVisibility::Hidden);

    UIInfo->State = EUIState::Hidden;
    RemoveFromStack(UIName);
    HandleStackChange();

    OnUIHidden.Broadcast(UIName);
    OnUITopChanged.Broadcast(GetTopUIName());
}

void UUIManager::CloseUI(FName UIName, bool bDestroyInstance, bool bRestorePreviousMainPanel)
{
    if (bIsShuttingDown) return;
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            CloseUIByWidget(UIInfo->WidgetInstance, bDestroyInstance, bRestorePreviousMainPanel);
        }
    }
}

void UUIManager::CloseUIByWidget(UUserWidget* Widget, bool bDestroyInstance, bool bRestorePreviousMainPanel)
{
    if (!Widget || bIsShuttingDown) return;
    FName UIName = GetUINameByWidget(Widget);
    if (UIName.IsNone()) return;

    FUIInfo* UIInfo = UIRegistry.Find(UIName);
    if (!UIInfo) return;

    bool bIsMainPanel = (UIInfo->PanelType == EUIPanelType::MainPanel);

    if (bIsMainPanel && bRestorePreviousMainPanel)
    {
        if (CurrentInputActiveUI.Get() == Widget)
        {
            if (CurrentInputActiveUI.IsValid())
                CurrentInputActiveUI->DeactivateInput(true);
            CurrentInputActiveUI = nullptr;
        }

        if (UUIBase* UIBase = Cast<UUIBase>(Widget))
            InternalCloseUI(UIBase);
        else
            SafeRemoveWidget(Widget);

        RemoveFromStack(UIName);

        if (bDestroyInstance)
        {
            WidgetToUINameMap.Remove(Widget);
            UIInfo->WidgetInstance = nullptr;
            FUIConfigData TempConfig;
            if (!UIConfigData || !UIConfigData->GetUIConfig(UIName, TempConfig))
            {
                UIRegistry.Remove(UIName);
            }
            else
            {
                UIInfo->State = EUIState::Hidden;
            }
        }
        else
        {
            UIInfo->State = EUIState::Hidden;
            UIInfo->WidgetInstance = nullptr;
            WidgetToUINameMap.Remove(Widget);
        }

        OnUIClosed.Broadcast(UIName);
        TryRestorePreviousMainPanel();
        HandleStackChange();
        OnUITopChanged.Broadcast(GetTopUIName());
        return;
    }

    if (CurrentInputActiveUI.Get() == Widget)
    {
        if (CurrentInputActiveUI.IsValid())
            CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }

    if (UUIBase* UIBase = Cast<UUIBase>(Widget))
        InternalCloseUI(UIBase);
    else
        SafeRemoveWidget(Widget);

    RemoveFromStack(UIName);
    HandleStackChange();

    if (bDestroyInstance)
    {
        WidgetToUINameMap.Remove(Widget);
        UIInfo->WidgetInstance = nullptr;
        FUIConfigData TempConfig;
        if (!UIConfigData || !UIConfigData->GetUIConfig(UIName, TempConfig))
        {
            UIRegistry.Remove(UIName);
        }
        else
        {
            UIInfo->State = EUIState::Hidden;
        }
    }
    else
    {
        UIInfo->State = EUIState::Hidden;
        UIInfo->WidgetInstance = nullptr;
        WidgetToUINameMap.Remove(Widget);
    }

    OnUIClosed.Broadcast(UIName);
    OnUITopChanged.Broadcast(GetTopUIName());
}

void UUIManager::CloseAllUI()
{
    if (bIsShuttingDown) return;
    ForceReleaseAllInputs();
    while (UIStack.Num() > 0)
    {
        CloseUI(UIStack.Last().UIName, true, false);
    }
    MainPanelHistoryStack.Empty();
}

void UUIManager::CloseTopUI()
{
    if (bIsShuttingDown) return;
    if (UIStack.Num() > 0)
        CloseUI(UIStack.Last().UIName, true, false);
}

void UUIManager::ShowUIByWidget(UUserWidget* Widget, UObject* Data)
{
    if (!Widget) return;
    FName UIName = GetUINameByWidget(Widget);
    if (UIName.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::ShowUIByWidget - Widget %s not registered"), *Widget->GetName());
        return;
    }
    ShowUIByName(UIName, Data, EUIOpenPolicy::Additive);
}

// ========== 内部实现（不触发公共事件，仅操作UI） ==========
void UUIManager::InternalShowUI(UUIBase* UI, UObject* Data)
{
    if (!UI) return;
    UI->InternalShowUI(Data);
}

void UUIManager::InternalHideUI(UUIBase* UI)
{
    if (!UI) return;
    UI->InternalHideUI();
}

void UUIManager::InternalCloseUI(UUIBase* UI)
{
    if (!UI) return;
    UI->InternalCloseUI();
}

void UUIManager::InternalHideUISilent(UUIBase* UI)
{
    if (!UI) return;
    UI->SetVisibility(ESlateVisibility::Hidden);
    if (CurrentInputActiveUI.Get() == UI)
    {
        if (CurrentInputActiveUI.IsValid())
            CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }
}

// ========== MainPanel 专用处理 ==========
void UUIManager::HandleMainPanelShow(FName NewMainPanelName)
{
    FName CurrentMainPanel = NAME_None;
    for (int32 i = UIStack.Num() - 1; i >= 0; --i)
    {
        FUIInfo* Info = UIRegistry.Find(UIStack[i].UIName);
        if (Info && Info->PanelType == EUIPanelType::MainPanel && Info->State == EUIState::Visible)
        {
            CurrentMainPanel = UIStack[i].UIName;
            break;
        }
    }

    if (CurrentMainPanel != NAME_None && CurrentMainPanel != NewMainPanelName)
    {
        HandleMainPanelHide(CurrentMainPanel, true);
    }
}

void UUIManager::HandleMainPanelHide(FName HiddenMainPanelName, bool bPushToHistory)
{
    FUIInfo* Info = UIRegistry.Find(HiddenMainPanelName);
    if (!Info || !Info->WidgetInstance) return;

    if (bPushToHistory)
    {
        if (MainPanelHistoryStack.Num() == 0 || MainPanelHistoryStack.Last() != HiddenMainPanelName)
        {
            MainPanelHistoryStack.Add(HiddenMainPanelName);
        }
    }

    if (UUIBase* UIBase = Cast<UUIBase>(Info->WidgetInstance))
        InternalHideUISilent(UIBase);
    else
        Info->WidgetInstance->SetVisibility(ESlateVisibility::Hidden);

    Info->State = EUIState::Hidden;
    RemoveFromStack(HiddenMainPanelName);
}

bool UUIManager::TryRestorePreviousMainPanel()
{
    if (MainPanelHistoryStack.Num() == 0) return false;

    FName PreviousMainPanel = MainPanelHistoryStack.Pop();
    if (PreviousMainPanel != NAME_None)
    {
        ShowUIByName(PreviousMainPanel, nullptr, EUIOpenPolicy::Additive);
        return true;
    }
    return false;
}

// ========== 栈管理 ==========
void UUIManager::AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer, bool bModifyInput, int32 Priority)
{
    RemoveFromStack(UIName);
    uint64 Order = NextOpenOrder++;
    UIStack.Add(FUILayerNode(UIName, Layer, Widget, bModifyInput, Priority, Order));
    UpdateStackOrder();
    RefreshAllZOrder();
}

void UUIManager::RemoveFromStack(FName UIName)
{
    for (int32 i = UIStack.Num() - 1; i >= 0; --i)
    {
        if (UIStack[i].UIName == UIName)
        {
            UIStack.RemoveAt(i);
            break;
        }
    }
    RefreshAllZOrder();
}

void UUIManager::UpdateStackOrder()
{
    UIStack.Sort([](const FUILayerNode& A, const FUILayerNode& B) {
        if (A.Layer != B.Layer)
            return static_cast<int32>(A.Layer) < static_cast<int32>(B.Layer);
        if (A.Priority != B.Priority)
            return A.Priority < B.Priority;
        return A.OpenOrder < B.OpenOrder;
        });
}

void UUIManager::RefreshAllZOrder()
{
    // 根据排序后的栈顺序，重新设置每个可见Widget的ZOrder
    for (int32 i = 0; i < UIStack.Num(); ++i)
    {
        UUserWidget* Widget = UIStack[i].Widget;
        if (Widget && Widget->IsInViewport())
        {
            // 移除后重新添加来更新ZOrder
            Widget->RemoveFromParent();
            Widget->AddToViewport(i);  // 使用索引作为ZOrder
        }
    }
}

void UUIManager::SafeRemoveWidget(UUserWidget* Widget)
{
    if (IsValid(Widget) && Widget->IsInViewport())
        Widget->RemoveFromParent();
}

// ========== 输入管理 ==========
void UUIManager::UpdateTopUIInput()
{
    HandleStackChange();
}

UUIBase* UUIManager::GetCurrentInputActiveUI() const
{
    return CurrentInputActiveUI.Get();
}

void UUIManager::ForceReleaseAllInputs()
{
    if (bIsShuttingDown) return;
    APlayerController* PlayerController = GetPlayerController();
    if (PlayerController)
        PlayerController->FlushPressedKeys();

    if (CurrentInputActiveUI.IsValid())
    {
        CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }
}

void UUIManager::DeactivatePreviousUIInput()
{
    if (CurrentInputActiveUI.IsValid())
    {
        CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }
}

void UUIManager::ActivateTopUIInput()
{
    HandleStackChange();
}

void UUIManager::HandleStackChange()
{
    if (bIsShuttingDown) return;

    APlayerController* PC = GetPlayerController();
    AXyPlayerControllerBase* XyPC = Cast<AXyPlayerControllerBase>(PC);
    if (!XyPC)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::HandleStackChange - PlayerController is not of type AXyPlayerControllerBase"));
        return;
    }

    // 寻找需要激活的顶层 UI
    UUIBase* DesiredUI = nullptr;
    for (int32 i = UIStack.Num() - 1; i >= 0; --i)
    {
        FUILayerNode& Node = UIStack[i];
        if (Node.Widget && Node.Widget->IsVisible() && Node.bModifyInput)
        {
            UUIBase* Candidate = Cast<UUIBase>(Node.Widget);
            if (Candidate && Candidate->GetInputMode() != EUIInputMode::GameOnly)
            {
                DesiredUI = Candidate;
                break;
            }
        }
    }

    // 计算期望的输入模式和焦点控件
    EUIInputMode DesiredMode = EUIInputMode::GameOnly;
    UUserWidget* DesiredFocus = nullptr;
    bool bDesiredShowMouse = false;

    if (DesiredUI)
    {
        DesiredMode = DesiredUI->GetInputMode();
        bDesiredShowMouse = DesiredUI->ShouldShowMouseCursor();
        DesiredFocus = FindFirstFocusableWidget(DesiredUI);
        if (!DesiredFocus) DesiredFocus = DesiredUI;
    }

    // 检查是否需要切换
    bool bNeedSwitch = (DesiredMode != CurrentAppliedInputMode);
    if (!bNeedSwitch && DesiredMode != EUIInputMode::GameOnly)
    {
        if (DesiredFocus != CurrentAppliedFocusWidget.Get() || bDesiredShowMouse != bCurrentAppliedShowMouse)
            bNeedSwitch = true;
    }
    else if (!bNeedSwitch && DesiredMode == EUIInputMode::GameOnly)
    {
        if (bDesiredShowMouse != bCurrentAppliedShowMouse)
            bNeedSwitch = true;
    }

    if (!bNeedSwitch)
    {
        if (DesiredUI && DesiredUI != CurrentInputActiveUI.Get())
        {
            if (CurrentInputActiveUI.IsValid())
                CurrentInputActiveUI->DeactivateInput(true);
            DesiredUI->ActivateInput();
            CurrentInputActiveUI = DesiredUI;
        }
        else if (!DesiredUI && CurrentInputActiveUI.IsValid())
        {
            CurrentInputActiveUI->DeactivateInput(true);
            CurrentInputActiveUI = nullptr;
        }
        return;
    }

    // 执行切换
    if (CurrentInputActiveUI.IsValid())
    {
        CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }

    if (DesiredUI)
    {
        DesiredUI->ActivateInput();
        CurrentInputActiveUI = DesiredUI;
    }

    // 调用 PlayerController 设置输入模式
    XyPC->SetUIInputMode(DesiredMode, DesiredFocus, bDesiredShowMouse, true);

    // 设置焦点
    if (DesiredFocus && DesiredMode != EUIInputMode::GameOnly)
    {
        DesiredFocus->SetFocus();
    }

    // 更新缓存
    CurrentAppliedInputMode = DesiredMode;
    CurrentAppliedFocusWidget = DesiredFocus;
    bCurrentAppliedShowMouse = bDesiredShowMouse;

    UE_LOG(LogTemp, Log, TEXT("HandleStackChange - Switched to Mode: %s, UI: %s"),
        *UEnum::GetValueAsString(DesiredMode),
        DesiredUI ? *DesiredUI->GetName() : TEXT("None"));
}

// ========== 查询方法 ==========
UUserWidget* UUIManager::GetUI(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
        return UIInfo->WidgetInstance;
    return nullptr;
}

bool UUIManager::IsUIVisible(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
        return UIInfo->State == EUIState::Visible && UIInfo->WidgetInstance != nullptr;
    return false;
}

bool UUIManager::DoesUIExist(FName UIName) const
{
    return UIRegistry.Contains(UIName);
}

TArray<FName> UUIManager::GetAllActiveUIs() const
{
    TArray<FName> ActiveUIs;
    for (const auto& Pair : UIRegistry)
    {
        if (Pair.Value.WidgetInstance && Pair.Value.State == EUIState::Visible)
            ActiveUIs.Add(Pair.Key);
    }
    return ActiveUIs;
}

TArray<FName> UUIManager::GetAllRegisteredUIs() const
{
    TArray<FName> UINames;
    UIRegistry.GetKeys(UINames);
    return UINames;
}

FName UUIManager::GetUINameByWidget(UUserWidget* Widget) const
{
    if (const FName* FoundName = WidgetToUINameMap.Find(Widget))
        return *FoundName;
    return NAME_None;
}

void UUIManager::SetUILayer(FName UIName, EUIPanelLayer NewLayer)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        UIInfo->Layer = NewLayer;
        for (FUILayerNode& Node : UIStack)
        {
            if (Node.UIName == UIName)
            {
                Node.Layer = NewLayer;
                break;
            }
        }
        UpdateStackOrder();
        RefreshAllZOrder();
        HandleStackChange();
    }
}

EUIPanelLayer UUIManager::GetUILayer(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
        return UIInfo->Layer;
    return EUIPanelLayer::None;
}

FName UUIManager::GetTopUIName() const
{
    return UIStack.Num() > 0 ? UIStack.Last().UIName : NAME_None;
}

UUserWidget* UUIManager::GetTopUI() const
{
    return UIStack.Num() > 0 ? UIStack.Last().Widget : nullptr;
}

int32 UUIManager::GetUIStackDepth() const
{
    return UIStack.Num();
}

void UUIManager::ClearUIStack()
{
    if (bIsShuttingDown)
    {
        UIStack.Empty();
        return;
    }

    ForceReleaseAllInputs();
    for (const FUILayerNode& Node : UIStack)
    {
        SafeRemoveWidget(Node.Widget);
    }
    UIStack.Empty();
    MainPanelHistoryStack.Empty();
}

// ========== 辅助方法 ==========
APlayerController* UUIManager::GetPlayerController() const
{
    UWorld* World = GetWorld();
    return World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
}

UWorld* UUIManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
                return Context.World();
        }
    }
    return nullptr;
}

UUserWidget* UUIManager::FindFirstFocusableWidget(UWidget* RootWidget) const
{
    if (!RootWidget) return nullptr;

    if (UUserWidget* UserWid = Cast<UUserWidget>(RootWidget))
    {
        if (UserWid->HasAnyUserFocus())
            return UserWid;
    }

    if (UPanelWidget* Panel = Cast<UPanelWidget>(RootWidget))
    {
        for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
        {
            UWidget* Child = Panel->GetChildAt(i);
            if (UUserWidget* Found = FindFirstFocusableWidget(Child))
                return Found;
        }
    }

    return Cast<UUserWidget>(RootWidget);
}

void UUIManager::CloseAllUIsOfType(EUIPanelType Type)
{
    TArray<FName> ToClose;
    for (const auto& Pair : UIRegistry)
    {
        if (Pair.Value.PanelType == Type && Pair.Value.WidgetInstance)
            ToClose.Add(Pair.Key);
    }
    for (FName UIName : ToClose)
        CloseUI(UIName, true, false);
}

// ========== 调试 ==========
void UUIManager::PrintAllUIs()
{
    UE_LOG(LogTemp, Log, TEXT("=== All Registered UIs ==="));
    for (const auto& Pair : UIRegistry)
    {
        const FUIInfo& UIInfo = Pair.Value;
        UE_LOG(LogTemp, Log, TEXT("UI: %s, Class: %s, Layer: %s, State: %s, Instance: %s, Preloaded: %s, PanelType: %s, ModifyInput: %s, Priority: %d, ClosePreviousMainPanel: %s"),
            *UIInfo.UIName.ToString(), UIInfo.WidgetClass ? *UIInfo.WidgetClass->GetName() : TEXT("Null"),
            *UEnum::GetValueAsString(UIInfo.Layer), *UEnum::GetValueAsString(UIInfo.State),
            UIInfo.WidgetInstance ? TEXT("Valid") : TEXT("Null"), UIInfo.bIsPreloaded ? TEXT("Yes") : TEXT("No"),
            *UEnum::GetValueAsString(UIInfo.PanelType), UIInfo.bModifyInput ? TEXT("true") : TEXT("false"),
            UIInfo.Priority, UIInfo.bClosePreviousMainPanel ? TEXT("true") : TEXT("false"));
    }
    UE_LOG(LogTemp, Log, TEXT("=========================="));
}

void UUIManager::PrintStackInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== UI Stack Information ==="));
    UE_LOG(LogTemp, Log, TEXT("Stack Depth: %d"), UIStack.Num());
    UE_LOG(LogTemp, Log, TEXT("MainPanel History Depth: %d"), MainPanelHistoryStack.Num());
    UE_LOG(LogTemp, Log, TEXT("Current Input Active UI: %s"), CurrentInputActiveUI.IsValid() ? *CurrentInputActiveUI->GetName() : TEXT("None"));
    for (int32 i = UIStack.Num() - 1; i >= 0; i--)
    {
        const FUILayerNode& Node = UIStack[i];
        UE_LOG(LogTemp, Log, TEXT("[%d] Name: %s, Layer: %s, Priority: %d, OpenOrder: %llu, Widget: %s, ModifyInput: %s"),
            i, *Node.UIName.ToString(), *UEnum::GetValueAsString(Node.Layer), Node.Priority, Node.OpenOrder,
            Node.Widget ? TEXT("Valid") : TEXT("Null"), Node.bModifyInput ? TEXT("true") : TEXT("false"));
    }
    UE_LOG(LogTemp, Log, TEXT("MainPanel History:"));
    for (int32 i = MainPanelHistoryStack.Num() - 1; i >= 0; --i)
        UE_LOG(LogTemp, Log, TEXT("  [%d] %s"), i, *MainPanelHistoryStack[i].ToString());
    UE_LOG(LogTemp, Log, TEXT("============================="));
}

void UUIManager::PrintConfigInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== UI Config Information ==="));
    if (UIConfigData)
    {
        TArray<FUIConfigData> AllConfigs = UIConfigData->GetAllUIConfigs();
        UE_LOG(LogTemp, Log, TEXT("Config Data Asset: %s"), *UIConfigData->GetName());
        UE_LOG(LogTemp, Log, TEXT("Total Config Entries: %d"), AllConfigs.Num());
        for (const FUIConfigData& Config : AllConfigs)
        {
            UE_LOG(LogTemp, Log, TEXT("  UI: %s, Class: %s, Layer: %s, InputMode: %s, Preload: %s, PanelType: %s, ModifyInput: %s, Priority: %d, ClosePreviousMainPanel: %s, Desc: %s"),
                *Config.UIName.ToString(), *Config.WidgetClass.ToString(), *UEnum::GetValueAsString(Config.DefaultLayer),
                *UEnum::GetValueAsString(Config.DefaultInputMode), Config.bPreload ? TEXT("Yes") : TEXT("No"),
                *UEnum::GetValueAsString(Config.PanelType), Config.bModifyInput ? TEXT("true") : TEXT("false"),
                Config.Priority, Config.bClosePreviousMainPanel ? TEXT("true") : TEXT("false"), *Config.Description);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No config data asset set"));
    }
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

// ========== 语言切换支持 ==========
void UUIManager::OnLanguageChanged(ELanguageType NewLanguage)
{
    UE_LOG(LogTemp, Log, TEXT("UIManager: Language changed to %s, refreshing active UIs..."),
        *UxyLanguageManager::GetLanguageManager()->GetLanguageCode(NewLanguage));
    RefreshAllActiveUIs();
}

void UUIManager::RefreshAllActiveUIs()
{
    if (bIsShuttingDown) return;

    TArray<FName> ActiveUINames;
    for (const auto& Pair : UIRegistry)
    {
        if (Pair.Value.WidgetInstance && Pair.Value.State == EUIState::Visible)
            ActiveUINames.Add(Pair.Key);
    }

    TMap<FName, UObject*> UIDataMap;
    for (FName UIName : ActiveUINames)
    {
        if (UUIBase* UI = Cast<UUIBase>(GetUI(UIName)))
        {
            UIDataMap.Add(UIName, UI->GetCurrentData());
        }
    }

    for (FName UIName : ActiveUINames)
    {
        CloseUI(UIName, true, false);
    }

    for (FName UIName : ActiveUINames)
    {
        UObject* Data = UIDataMap.FindRef(UIName);
        ShowUIByName(UIName, Data, EUIOpenPolicy::Additive);
    }
}