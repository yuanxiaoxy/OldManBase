// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIManager.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
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
    if (ConfigDataAsset)
    {
        LoadUIConfig(ConfigDataAsset);
    }
}

void UUIManager::InitializeSingleton()
{
    // 初始化代码（如果需要）
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

bool UUIManager::RegisterUIFromConfig(const FUIConfigData& Config)
{
    if (Config.UIName.IsNone() || Config.WidgetClass.IsNull()) return false;
    if (UIRegistry.Contains(Config.UIName)) return false;

    TSubclassOf<UUserWidget> WidgetClass = LoadWidgetClass(Config.WidgetClass);
    if (!WidgetClass) return false;

    FUIInfo UIInfo;
    UIInfo.UIName = Config.UIName;
    UIInfo.WidgetClass = WidgetClass;
    UIInfo.Layer = Config.DefaultLayer;
    UIInfo.State = EUIState::Hidden;
    UIInfo.WidgetInstance = nullptr;
    UIInfo.bIsPreloaded = Config.bPreload;
    UIRegistry.Add(Config.UIName, UIInfo);
    return true;
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
                    if (UUIBase* UIBase = Cast<UUIBase>(Widget))
                    {
                        FUIConfigData Config;
                        if (UIConfigData && UIConfigData->GetUIConfig(UIName, Config))
                        {
                            UIBase->SetInputMode(Config.DefaultInputMode);
                            UIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
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
        if (FUIInfo* UIInfo = UIRegistry.Find(Config.UIName))
        {
            if (!UIInfo->WidgetInstance && UIInfo->WidgetClass)
            {
                APlayerController* PlayerController = GetPlayerController();
                if (!PlayerController) continue;
                UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, UIInfo->WidgetClass);
                if (Widget)
                {
                    UIInfo->WidgetInstance = Widget;
                    UIInfo->bIsPreloaded = true;
                    if (UUIBase* UIBase = Cast<UUIBase>(Widget))
                    {
                        UIBase->SetInputMode(Config.DefaultInputMode);
                        UIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
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

    FUIConfigData Config;
    bool bHasConfig = UIConfigData && UIConfigData->GetUIConfig(UIName, Config);
    EUIPanelType PanelType = bHasConfig ? Config.PanelType : EUIPanelType::Other;

    UE_LOG(LogTemp, Log, TEXT("UUIManager::ShowUI - Showing UI: %s, PanelType: %s, InputMode: %s"),
        *UIName.ToString(), *UEnum::GetValueAsString(PanelType), bHasConfig ? *UEnum::GetValueAsString(Config.DefaultInputMode) : TEXT("None"));

    // 处理MainPanel类型：自动隐藏其他所有MainPanel，并记录到历史栈
    if (PanelType == EUIPanelType::MainPanel)
    {
        TArray<FName> AllActiveUIs = GetAllActiveUIs();
        for (FName ActiveUIName : AllActiveUIs)
        {
            if (ActiveUIName == UIName) continue;
            FUIConfigData ActiveConfig;
            if (UIConfigData && UIConfigData->GetUIConfig(ActiveUIName, ActiveConfig) &&
                ActiveConfig.PanelType == EUIPanelType::MainPanel)
            {
                UE_LOG(LogTemp, Log, TEXT("UUIManager::ShowUI - Hiding other MainPanel: %s"), *ActiveUIName.ToString());
                // 将被隐藏的 MainPanel 压入历史栈
                MainPanelHistoryStack.Add(ActiveUIName);
                HideUI(ActiveUIName);
            }
        }
    }

    // 打开策略处理
    if (OpenPolicy == EUIOpenPolicy::Replace)
    {
        if (UIStack.Num() > 0)
            CloseUI(UIStack.Last().UIName, true);
    }
    else if (OpenPolicy == EUIOpenPolicy::Exclusive)
    {
        TArray<FName> AllUIs = GetAllActiveUIs();
        for (FName Name : AllUIs)
        {
            if (Name != UIName)
                CloseUI(Name, true);
        }
    }

    // 检查是否已存在
    if (FUIInfo* ExistingUI = UIRegistry.Find(UIName))
    {
        if (ExistingUI->WidgetInstance)
        {
            // 如果配置存在，重新应用输入设置（确保模式正确）
            if (bHasConfig && ExistingUI->WidgetInstance->IsA<UUIBase>())
            {
                UUIBase* UIBase = Cast<UUIBase>(ExistingUI->WidgetInstance);
                UIBase->SetInputMode(Config.DefaultInputMode);
                UIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
                if (Config.DefaultInputMappingContext)
                {
                    UInputMappingContext* IMC = LoadInputMappingContext(Config.DefaultInputMappingContext);
                    if (IMC)
                        UIBase->SetInputMappingContext(IMC, Config.InputPriority);
                }
                UE_LOG(LogTemp, Log, TEXT("UUIManager::ShowUI - Reapplied input settings for existing UI: %s"), *UIName.ToString());
            }

            if (ExistingUI->State != EUIState::Visible)
            {
                AddToStack(ExistingUI->WidgetInstance, UIName, ExistingUI->Layer);
                HandleStackChange();

                if (UUIBase* UIBase = Cast<UUIBase>(ExistingUI->WidgetInstance))
                    UIBase->ShowUI(Data);
                if (ExistingUI->WidgetInstance->GetVisibility() != ESlateVisibility::Visible)
                    ExistingUI->WidgetInstance->SetVisibility(ESlateVisibility::Visible);
                ExistingUI->State = EUIState::Visible;
            }
            else
            {
                RemoveFromStack(UIName);
                AddToStack(ExistingUI->WidgetInstance, UIName, ExistingUI->Layer);
                HandleStackChange();
            }
            if (Data && Cast<UUIBase>(ExistingUI->WidgetInstance))
                Cast<UUIBase>(ExistingUI->WidgetInstance)->SetData(Data);
            OnUIShown.Broadcast(UIName);
            return ExistingUI->WidgetInstance;
        }
    }

    // 创建新UI
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
    if (!NewWidget) return nullptr;

    if (!UIRegistry.Contains(UIName))
    {
        FUIInfo UIInfo;
        UIInfo.UIName = UIName;
        UIInfo.WidgetClass = WidgetClass;
        UIInfo.Layer = Layer;
        UIInfo.State = EUIState::Visible;
        UIInfo.WidgetInstance = NewWidget;
        UIInfo.bIsPreloaded = false;
        UIRegistry.Add(UIName, UIInfo);
    }
    else
    {
        FUIInfo* UIInfo = UIRegistry.Find(UIName);
        UIInfo->WidgetInstance = NewWidget;
        UIInfo->State = EUIState::Visible;
        UIInfo->Layer = Layer;
    }

    // 应用配置
    if (UUIBase* UIBase = Cast<UUIBase>(NewWidget))
    {
        if (bHasConfig)
        {
            UIBase->SetInputMode(Config.DefaultInputMode);
            UIBase->bShowMouseCursorWhenActive = Config.bShowMouseCursor;
            if (Config.DefaultInputMappingContext)
            {
                UInputMappingContext* IMC = LoadInputMappingContext(Config.DefaultInputMappingContext);
                if (IMC)
                    UIBase->SetInputMappingContext(IMC, Config.InputPriority);
            }
            UE_LOG(LogTemp, Log, TEXT("UUIManager::ShowUI - Applied input settings for new UI: %s, InputMode: %s"),
                *UIName.ToString(), *UEnum::GetValueAsString(Config.DefaultInputMode));
        }
    }

    AddToStack(NewWidget, UIName, Layer);
    HandleStackChange();

    if (UUIBase* UIBase = Cast<UUIBase>(NewWidget))
        UIBase->ShowUI(Data);

    if (!NewWidget->IsInViewport())
        NewWidget->AddToViewport();
    else
        UE_LOG(LogTemp, Warning, TEXT("UUIManager::ShowUI - Widget %s already in viewport, skipped AddToViewport."), *UIName.ToString());

    NewWidget->SetVisibility(ESlateVisibility::Visible);

    OnUIShown.Broadcast(UIName);
    OnUITopChanged.Broadcast(UIName);
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

    // 检查是否是 MainPanel
    bool bIsMainPanel = false;
    FUIConfigData Config;
    if (UIConfigData && UIConfigData->GetUIConfig(UIName, Config))
    {
        bIsMainPanel = (Config.PanelType == EUIPanelType::MainPanel);
    }

    // 如果需要恢复上一个 MainPanel，且是 MainPanel，则从历史栈中弹出（如果栈非空）
    FName PreviousMainPanel = NAME_None;
    if (bRestorePreviousMainPanel && bIsMainPanel && MainPanelHistoryStack.Num() > 0)
    {
        PreviousMainPanel = MainPanelHistoryStack.Pop();
    }

    // 执行隐藏逻辑
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            if (UUIBase* UIBase = Cast<UUIBase>(UIInfo->WidgetInstance))
                UIBase->HideUI();
            else
                UIInfo->WidgetInstance->SetVisibility(ESlateVisibility::Hidden);
            UIInfo->State = EUIState::Hidden;
            RemoveFromStack(UIName);
            HandleStackChange();
            OnUIHidden.Broadcast(UIName);
            OnUITopChanged.Broadcast(GetTopUIName());
        }
    }

    // 恢复上一个 MainPanel
    if (PreviousMainPanel != NAME_None)
    {
        UE_LOG(LogTemp, Log, TEXT("UUIManager::HideUI - Restoring previous MainPanel: %s"), *PreviousMainPanel.ToString());
        ShowUIByName(PreviousMainPanel);
    }
}

void UUIManager::CloseUI(FName UIName, bool bDestroyInstance, bool bRestorePreviousMainPanel)
{
    if (bIsShuttingDown) return;

    // 检查是否是 MainPanel
    bool bIsMainPanel = false;
    FUIConfigData Config;
    if (UIConfigData && UIConfigData->GetUIConfig(UIName, Config))
    {
        bIsMainPanel = (Config.PanelType == EUIPanelType::MainPanel);
    }

    // 如果需要恢复上一个 MainPanel，且是 MainPanel，则从历史栈中弹出（如果栈非空）
    FName PreviousMainPanel = NAME_None;
    if (bRestorePreviousMainPanel && bIsMainPanel && MainPanelHistoryStack.Num() > 0)
    {
        PreviousMainPanel = MainPanelHistoryStack.Pop();
    }

    // 执行关闭逻辑
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            if (CurrentInputActiveUI.Get() == UIInfo->WidgetInstance)
            {
                if (CurrentInputActiveUI.IsValid())
                {
                    CurrentInputActiveUI->DeactivateInput(true);
                }
                CurrentInputActiveUI = nullptr;
            }
            if (UUIBase* UIBase = Cast<UUIBase>(UIInfo->WidgetInstance))
                UIBase->CloseUI();
            else
                SafeRemoveWidget(UIInfo->WidgetInstance);
            RemoveFromStack(UIName);
            HandleStackChange();

            if (bDestroyInstance)
            {
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
            }
            OnUIClosed.Broadcast(UIName);
            OnUITopChanged.Broadcast(GetTopUIName());
        }
    }

    // 恢复上一个 MainPanel
    if (PreviousMainPanel != NAME_None)
    {
        UE_LOG(LogTemp, Log, TEXT("UUIManager::CloseUI - Restoring previous MainPanel: %s"), *PreviousMainPanel.ToString());
        ShowUIByName(PreviousMainPanel);
    }
}

void UUIManager::CloseAllUI()
{
    if (bIsShuttingDown) return;
    ForceReleaseAllInputs();
    while (UIStack.Num() > 0)
    {
        CloseUI(UIStack.Last().UIName, true);
    }
    MainPanelHistoryStack.Empty();
}

void UUIManager::CloseTopUI()
{
    if (bIsShuttingDown) return;
    if (UIStack.Num() > 0)
        CloseUI(UIStack.Last().UIName, true);
}

void UUIManager::UpdateTopUIInput()
{
    if (bIsShuttingDown) return;
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
    {
        PlayerController->FlushPressedKeys();
    }

    if (CurrentInputActiveUI.IsValid())
    {
        CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }
}

void UUIManager::DeactivatePreviousUIInput()
{
    if (bIsShuttingDown) return;
    if (CurrentInputActiveUI.IsValid())
    {
        CurrentInputActiveUI->DeactivateInput(true);
        CurrentInputActiveUI = nullptr;
    }
}

void UUIManager::ActivateTopUIInput()
{
    if (bIsShuttingDown) return;
    if (UIStack.Num() > 0)
    {
        FUILayerNode TopNode = UIStack.Last();
        if (TopNode.Widget && TopNode.Widget->IsVisible())
        {
            UUIBase* TopUIBase = Cast<UUIBase>(TopNode.Widget);
            if (TopUIBase && TopUIBase->GetInputMode() != EUIInputMode::GameOnly)
            {
                TopUIBase->ActivateInput();
                CurrentInputActiveUI = TopUIBase;
            }
        }
    }
}

void UUIManager::HandleStackChange()
{
    if (bIsShuttingDown) return;

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

    UUIBase* TopUI = nullptr;
    bool bHasVisibleUI = false;
    EUIInputMode TopUIMode = EUIInputMode::GameOnly;
    bool bShowMouse = false;
    UUserWidget* FocusWidget = nullptr;

    for (int32 i = UIStack.Num() - 1; i >= 0; i--)
    {
        FUILayerNode& Node = UIStack[i];
        if (Node.Widget)
        {
            TopUI = Cast<UUIBase>(Node.Widget);
            bHasVisibleUI = true;
            break;
        }
    }

    if (bHasVisibleUI && TopUI)
    {
        TopUIMode = TopUI->GetInputMode();
        bShowMouse = TopUI->ShouldShowMouseCursor();

        UE_LOG(LogTemp, Log, TEXT("UUIManager::HandleStackChange - TopUI: %s, InputMode: %s, ShowMouse: %s"),
            *TopUI->GetName(), *UEnum::GetValueAsString(TopUIMode), bShowMouse ? TEXT("true") : TEXT("false"));

        TopUI->ActivateInput();
        CurrentInputActiveUI = TopUI;

        // 查找可聚焦控件
        UWidget* FoundWidget = FindFirstFocusableWidget(TopUI);
        if (FoundWidget && FoundWidget->IsA<UUserWidget>())
        {
            FocusWidget = Cast<UUserWidget>(FoundWidget);
        }
        else
        {
            FocusWidget = TopUI;
        }

        XyPC->SetUIInputMode(TopUIMode, FocusWidget, bShowMouse, true);
        if (FocusWidget)
            FocusWidget->SetFocus();

        UE_LOG(LogTemp, Log, TEXT("UUIManager::HandleStackChange - Set UI Input Mode to %s for UI: %s, FocusWidget: %s"),
            *UEnum::GetValueAsString(TopUIMode), *TopUI->GetName(), FocusWidget ? *FocusWidget->GetName() : TEXT("None"));
    }
    else
    {
        XyPC->SetUIInputMode(EUIInputMode::GameOnly, nullptr, false, true);
        UE_LOG(LogTemp, Log, TEXT("UUIManager::HandleStackChange - No visible UI, set GameOnly"));
    }
}

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
}

void UUIManager::AddToStack(UUserWidget* Widget, FName UIName, EUIPanelLayer Layer)
{
    RemoveFromStack(UIName);
    UIStack.Add(FUILayerNode(UIName, Layer, Widget));
    UpdateStackOrder();
}

void UUIManager::RemoveFromStack(FName UIName)
{
    for (int32 i = UIStack.Num() - 1; i >= 0; i--)
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
    if (IsValid(Widget))
    {
        if (Widget->IsInViewport())
            Widget->RemoveFromParent();
    }
}

void UUIManager::PrintAllUIs()
{
    UE_LOG(LogTemp, Log, TEXT("=== All Registered UIs ==="));
    for (const auto& Pair : UIRegistry)
    {
        const FUIInfo& UIInfo = Pair.Value;
        UE_LOG(LogTemp, Log, TEXT("UI: %s, Class: %s, Layer: %s, State: %s, Instance: %s, Preloaded: %s"),
            *UIInfo.UIName.ToString(), *UIInfo.WidgetClass->GetName(),
            *UEnum::GetValueAsString(UIInfo.Layer), *UEnum::GetValueAsString(UIInfo.State),
            UIInfo.WidgetInstance ? TEXT("Valid") : TEXT("Null"), UIInfo.bIsPreloaded ? TEXT("Yes") : TEXT("No"));
    }
    UE_LOG(LogTemp, Log, TEXT("=========================="));
}

void UUIManager::PrintStackInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== UI Stack Information ==="));
    UE_LOG(LogTemp, Log, TEXT("Stack Depth: %d"), UIStack.Num());
    UE_LOG(LogTemp, Log, TEXT("Current Input Active UI: %s"), CurrentInputActiveUI.IsValid() ? *CurrentInputActiveUI->GetName() : TEXT("None"));
    for (int32 i = UIStack.Num() - 1; i >= 0; i--)
    {
        const FUILayerNode& Node = UIStack[i];
        UE_LOG(LogTemp, Log, TEXT("[%d] Name: %s, Layer: %s, Widget: %s"), i, *Node.UIName.ToString(),
            *UEnum::GetValueAsString(Node.Layer), Node.Widget ? TEXT("Valid") : TEXT("Null"));
    }
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
            UE_LOG(LogTemp, Log, TEXT("  UI: %s, Class: %s, Layer: %s, InputMode: %s, Preload: %s, PanelType: %s, Desc: %s"),
                *Config.UIName.ToString(), *Config.WidgetClass.ToString(), *UEnum::GetValueAsString(Config.DefaultLayer),
                *UEnum::GetValueAsString(Config.DefaultInputMode), Config.bPreload ? TEXT("Yes") : TEXT("No"),
                *UEnum::GetValueAsString(Config.PanelType), *Config.Description);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No config data asset set"));
    }
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

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

UWidget* UUIManager::FindFirstFocusableWidget(UWidget* RootWidget) const
{
    if (!RootWidget) return nullptr;
    if (RootWidget->HasAnyUserFocus()) return RootWidget;

    if (UPanelWidget* Panel = Cast<UPanelWidget>(RootWidget))
    {
        for (int32 i = 0; i < Panel->GetChildrenCount(); ++i)
        {
            UWidget* Child = Panel->GetChildAt(i);
            if (UWidget* Found = FindFirstFocusableWidget(Child))
                return Found;
        }
    }
    return nullptr;
}