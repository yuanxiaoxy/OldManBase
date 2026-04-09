// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIManager.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UIManager/UIConfigDataAsset.h"
#include "UIManager/UIBase.h"
#include "XyCharacter/XyPlayerControllerBase.h"

template<>
UUIManager* TSingleton<UUIManager>::SingletonInstance = nullptr;

UUIManager::UUIManager()
    : bIsShuttingDown(false)
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

    TSubclassOf<UUserWidget> WidgetClass = LoadWidgetClass(Config.WidgetClass);
    if (!WidgetClass) return;

    FUIInfo UIInfo;
    UIInfo.UIName = Config.UIName;
    UIInfo.WidgetClass = WidgetClass;
    UIInfo.Layer = Config.DefaultLayer;
    UIInfo.State = EUIState::Hidden;
    UIInfo.WidgetInstance = nullptr;
    UIInfo.bIsPreloaded = Config.bPreload;
    UIInfo.PanelType = Config.PanelType;
    UIInfo.bModifyInput = Config.bModifyInput;
    UIRegistry.Add(Config.UIName, UIInfo);
}

TSubclassOf<UUserWidget> UUIManager::LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr)
{
    return SoftClassPtr.IsNull() ? nullptr : SoftClassPtr.LoadSynchronous();
}

UInputMappingContext* UUIManager::LoadInputMappingContext(const TSoftObjectPtr<UInputMappingContext>& SoftObjectPtr)
{
    return SoftObjectPtr.IsNull() ? nullptr : SoftObjectPtr.LoadSynchronous();
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
                UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, UIInfo->WidgetClass);
                if (Widget)
                {
                    UIInfo->WidgetInstance = Widget;
                    UIInfo->bIsPreloaded = true;
                    WidgetToUINameMap.Add(Widget, UIName);
                    if (UUIBase* UIBase = Cast<UUIBase>(Widget))
                    {
                        FUIConfigData Config;
                        if (UIConfigData && UIConfigData->GetUIConfig(UIName, Config))
                        {
                            UIBase->SetInputMode(Config.DefaultInputMode);
                            UIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
                            UIBase->PanelType = Config.PanelType;
                            UIBase->bModifyInput = Config.bModifyInput;
                            if (Config.DefaultInputMappingContext)
                            {
                                UInputMappingContext* IMC = LoadInputMappingContext(Config.DefaultInputMappingContext);
                                if (IMC)
                                    UIBase->SetInputMappingContext(IMC, Config.InputPriority);
                            }
                        }
                    }
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

UUserWidget* UUIManager::ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer, UObject* Data, FName UIName, EUIOpenPolicy OpenPolicy)
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
        // 重新入栈（改变顺序）
        RemoveFromStack(UIName);
        AddToStack(ExistingWidget, UIName, Layer, bModifyInput);
        HandleStackChange();

        if (ExistingUIBase && Data)
            ExistingUIBase->SetData(Data);

        OnUIShown.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return ExistingWidget;
    }

    // 处理 MainPanel 切换：如果要显示的是 MainPanel，先隐藏当前显示的 MainPanel
    if (PanelType == EUIPanelType::MainPanel)
    {
        HandleMainPanelShow(UIName);
    }

    // 如果已存在但未显示（Hidden），则直接显示
    if (ExistingWidget && UIInfo->State == EUIState::Hidden)
    {
        if (ExistingUIBase)
        {
            // 更新配置（可能运行时改变）
            if (bHasConfig)
            {
                ExistingUIBase->SetInputMode(Config.DefaultInputMode);
                ExistingUIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
                ExistingUIBase->PanelType = Config.PanelType;
                ExistingUIBase->bModifyInput = Config.bModifyInput;
                if (Config.DefaultInputMappingContext)
                {
                    UInputMappingContext* IMC = LoadInputMappingContext(Config.DefaultInputMappingContext);
                    if (IMC)
                        ExistingUIBase->SetInputMappingContext(IMC, Config.InputPriority);
                }
            }
            InternalShowUI(ExistingUIBase, Data);
        }
        else
        {
            ExistingWidget->SetVisibility(ESlateVisibility::Visible);
        }

        UIInfo->State = EUIState::Visible;
        AddToStack(ExistingWidget, UIName, Layer, bModifyInput);
        HandleStackChange();

        OnUIShown.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return ExistingWidget;
    }

    // 创建新UI
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
    if (!NewWidget) return nullptr;
    UUIBase* NewUIBase = Cast<UUIBase>(NewWidget);

    // 注册或更新注册表
    if (!UIInfo)
    {
        FUIInfo NewInfo;
        NewInfo.UIName = UIName;
        NewInfo.WidgetClass = WidgetClass;
        NewInfo.Layer = Layer;
        NewInfo.State = EUIState::Visible;
        NewInfo.WidgetInstance = NewWidget;
        NewInfo.bIsPreloaded = false;
        NewInfo.PanelType = PanelType;
        NewInfo.bModifyInput = bModifyInput;
        UIRegistry.Add(UIName, NewInfo);
    }
    else
    {
        UIInfo->WidgetInstance = NewWidget;
        UIInfo->State = EUIState::Visible;
        UIInfo->Layer = Layer;
        UIInfo->PanelType = PanelType;
        UIInfo->bModifyInput = bModifyInput;
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

    InternalShowUI(NewUIBase, Data);
    if (!NewWidget->IsInViewport())
        NewWidget->AddToViewport();

    AddToStack(NewWidget, UIName, Layer, bModifyInput);
    HandleStackChange();

    OnUIShown.Broadcast(UIName);
    OnUITopChanged.Broadcast(GetTopUIName());
    return NewWidget;
}

UUserWidget* UUIManager::ShowUIByName(FName UIName, UObject* Data, EUIOpenPolicy OpenPolicy)
{
    if (UIName.IsNone()) return nullptr;
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        return ShowUI(UIInfo->WidgetClass, UIInfo->Layer, Data, UIName, OpenPolicy);
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

    // 隐藏 MainPanel 并恢复上一个 MainPanel
    if (bIsMainPanel && bRestorePreviousMainPanel)
    {
        // 1. 隐藏当前 MainPanel（不销毁）
        if (UUIBase* UIBase = Cast<UUIBase>(Widget))
            InternalHideUI(UIBase);
        else
            Widget->SetVisibility(ESlateVisibility::Hidden);

        UIInfo->State = EUIState::Hidden;
        RemoveFromStack(UIName);

        // 2. 恢复上一个 MainPanel
        TryRestorePreviousMainPanel();

        HandleStackChange();
        OnUIHidden.Broadcast(UIName);
        OnUITopChanged.Broadcast(GetTopUIName());
        return;
    }

    // 普通隐藏
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

    // 处理 MainPanel 关闭并恢复上一个 MainPanel
    if (bIsMainPanel && bRestorePreviousMainPanel)
    {
        // 1. 先销毁/隐藏当前 MainPanel
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

        // 2. 恢复上一个 MainPanel（从历史栈中弹出并显示）
        TryRestorePreviousMainPanel();

        HandleStackChange();
        OnUITopChanged.Broadcast(GetTopUIName());
        return;
    }

    // 普通关闭（非 MainPanel 或不需要恢复）
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
    // 查找当前栈中可见的 MainPanel（可能只有一个）
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
        // 隐藏当前的 MainPanel，并推入历史栈
        HandleMainPanelHide(CurrentMainPanel, true);
    }
}

void UUIManager::HandleMainPanelHide(FName HiddenMainPanelName, bool bPushToHistory)
{
    FUIInfo* Info = UIRegistry.Find(HiddenMainPanelName);
    if (!Info || !Info->WidgetInstance) return;

    if (bPushToHistory)
    {
        // 避免重复推入同一个
        if (MainPanelHistoryStack.Num() == 0 || MainPanelHistoryStack.Last() != HiddenMainPanelName)
        {
            MainPanelHistoryStack.Add(HiddenMainPanelName);
        }
    }

    // 静默隐藏
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
        // 重新显示上一个 MainPanel，注意不要再次触发 MainPanel 切换循环
        ShowUIByName(PreviousMainPanel, nullptr, EUIOpenPolicy::Additive);
        return true;
    }
    return false;
}

// ========== 栈管理 ==========
void UUIManager::AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer, bool bModifyInput)
{
    RemoveFromStack(UIName);  // 确保不重复
    UIStack.Add(FUILayerNode(UIName, Layer, Widget, bModifyInput));
    UpdateStackOrder();
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
}

void UUIManager::UpdateStackOrder()
{
    UIStack.Sort([](const FUILayerNode& A, const FUILayerNode& B) {
        return static_cast<int32>(A.Layer) > static_cast<int32>(B.Layer);
        });
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
    for (int32 i = UIStack.Num() - 1; i >= 0; --i)
    {
        FUILayerNode& Node = UIStack[i];
        if (Node.Widget && Node.Widget->IsVisible() && Node.bModifyInput)
        {
            UUIBase* TopUIBase = Cast<UUIBase>(Node.Widget);
            if (TopUIBase && TopUIBase->GetInputMode() != EUIInputMode::GameOnly)
            {
                TopUIBase->ActivateInput();
                CurrentInputActiveUI = TopUIBase;
                return;
            }
        }
    }
}

void UUIManager::HandleStackChange()
{
    if (bIsShuttingDown) return;

    // 找到顶层可见且允许修改输入的 UI
    UUIBase* NewTopUI = nullptr;
    for (int32 i = UIStack.Num() - 1; i >= 0; --i)
    {
        FUILayerNode& Node = UIStack[i];
        if (Node.Widget && Node.Widget->IsVisible() && Node.bModifyInput)
        {
            NewTopUI = Cast<UUIBase>(Node.Widget);
            break;
        }
    }

    // 只有当找到了新的顶层 UI 且与当前激活的不同时，才切换输入状态
    if (NewTopUI && NewTopUI != CurrentInputActiveUI.Get())
    {
        // 停用之前的输入（如果存在）
        if (CurrentInputActiveUI.IsValid())
        {
            CurrentInputActiveUI->DeactivateInput(true);
            CurrentInputActiveUI = nullptr;
        }

        APlayerController* PC = GetPlayerController();
        AXyPlayerControllerBase* XyPC = Cast<AXyPlayerControllerBase>(PC);
        if (!XyPC)
        {
            UE_LOG(LogTemp, Error, TEXT("UUIManager::HandleStackChange - PlayerController is not of type AXyPlayerControllerBase"));
            return;
        }

        EUIInputMode TopUIMode = NewTopUI->GetInputMode();
        bool bShowMouse = NewTopUI->ShouldShowMouseCursor();
        NewTopUI->ActivateInput();
        CurrentInputActiveUI = NewTopUI;

        UUserWidget* FocusWidget = FindFirstFocusableWidget(NewTopUI);
        if (!FocusWidget) FocusWidget = NewTopUI;

        XyPC->SetUIInputMode(TopUIMode, FocusWidget, bShowMouse, true);
        if (FocusWidget) FocusWidget->SetFocus();

        UE_LOG(LogTemp, Log, TEXT("HandleStackChange - Activated UI: %s, Mode: %s"),
            *NewTopUI->GetName(), *UEnum::GetValueAsString(TopUIMode));
    }
    else if (!NewTopUI)
    {
        APlayerController* PC = GetPlayerController();
        AXyPlayerControllerBase* XyPC = Cast<AXyPlayerControllerBase>(PC);
        if (XyPC)
        {
            XyPC->SetUIInputMode(EUIInputMode::GameOnly, nullptr, false, true);
        }
        // 没有 bModifyInput=true 的可见 UI，不改变当前输入状态
        UE_LOG(LogTemp, Log, TEXT("HandleStackChange - No visible UI with bModifyInput=true, keeping current input mode."));
    }
    // 如果 NewTopUI 存在但等于 CurrentInputActiveUI，无需操作
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
        UE_LOG(LogTemp, Log, TEXT("UI: %s, Class: %s, Layer: %s, State: %s, Instance: %s, Preloaded: %s, PanelType: %s, ModifyInput: %s"),
            *UIInfo.UIName.ToString(), *UIInfo.WidgetClass->GetName(),
            *UEnum::GetValueAsString(UIInfo.Layer), *UEnum::GetValueAsString(UIInfo.State),
            UIInfo.WidgetInstance ? TEXT("Valid") : TEXT("Null"), UIInfo.bIsPreloaded ? TEXT("Yes") : TEXT("No"),
            *UEnum::GetValueAsString(UIInfo.PanelType), UIInfo.bModifyInput ? TEXT("true") : TEXT("false"));
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
        UE_LOG(LogTemp, Log, TEXT("[%d] Name: %s, Layer: %s, Widget: %s, ModifyInput: %s"),
            i, *Node.UIName.ToString(), *UEnum::GetValueAsString(Node.Layer),
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
            UE_LOG(LogTemp, Log, TEXT("  UI: %s, Class: %s, Layer: %s, InputMode: %s, Preload: %s, PanelType: %s, ModifyInput: %s, Desc: %s"),
                *Config.UIName.ToString(), *Config.WidgetClass.ToString(), *UEnum::GetValueAsString(Config.DefaultLayer),
                *UEnum::GetValueAsString(Config.DefaultInputMode), Config.bPreload ? TEXT("Yes") : TEXT("No"),
                *UEnum::GetValueAsString(Config.PanelType), Config.bModifyInput ? TEXT("true") : TEXT("false"),
                *Config.Description);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No config data asset set"));
    }
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}