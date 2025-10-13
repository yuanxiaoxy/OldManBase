// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIManager.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "UIManager/UIConfigDataAsset.h"
#include "UIManager/UIBase.h"

UUIManager::UUIManager()
{
    WorldContext = nullptr;
    RootCanvas = nullptr;
    UIConfigData = nullptr;
}

UUIManager::~UUIManager()
{
    CloseAllUI();
}

void UUIManager::InitializeUIManager(UUIConfigDataAsset* ConfigDataAsset)
{
    if (WorldContext == nullptr)
    {
        WorldContext = GetWorld();
    }

    CreateLayerPanels();

    // ��������
    if (ConfigDataAsset)
    {
        LoadUIConfig(ConfigDataAsset);
    }
}

void UUIManager::InitializeSingleton()
{
    //InitializeUIManager(nullptr);
}

void UUIManager::CreateLayerPanels()
{
    if (!WorldContext) return;

    // ����������
    RootCanvas = NewObject<UCanvasPanel>(GetTransientPackage(), UCanvasPanel::StaticClass());

    // �������㼶���
    for (int32 i = 0; i <= static_cast<int32>(EUIPanelLayer::ForeFront); i++)
    {
        EUIPanelLayer Layer = static_cast<EUIPanelLayer>(i);
        if (Layer == EUIPanelLayer::None) continue;

        UCanvasPanel* LayerPanel = NewObject<UCanvasPanel>(GetTransientPackage(), UCanvasPanel::StaticClass());
        LayerPanels.Add(Layer, LayerPanel);
    }
}

void UUIManager::LoadUIConfig(UUIConfigDataAsset* ConfigDataAsset)
{
    if (!ConfigDataAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::LoadUIConfig - ConfigDataAsset is null"));
        OnUIConfigLoaded.Broadcast(false);
        return;
    }

    UIConfigData = ConfigDataAsset;

    // ������ע������UI
    RegisterAllUIsFromConfig();

    UE_LOG(LogTemp, Log, TEXT("UUIManager::LoadUIConfig - Loaded UI config with %d entries"),
        UIRegistry.Num());

    OnUIConfigLoaded.Broadcast(true);
}

void UUIManager::ReloadUIConfig()
{
    if (UIConfigData)
    {
        // �ȹر�����UI
        CloseAllUI();

        // ���ע���
        UIRegistry.Empty();

        // ���¼�������
        RegisterAllUIsFromConfig();

        UE_LOG(LogTemp, Log, TEXT("UUIManager::ReloadUIConfig - Reloaded UI config"));
    }
}

void UUIManager::RegisterAllUIsFromConfig()
{
    if (!UIConfigData)
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIManager::RegisterAllUIsFromConfig - No config data available"));
        return;
    }

    TArray<FUIConfigData> AllConfigs = UIConfigData->GetAllUIConfigs();

    for (const FUIConfigData& Config : AllConfigs)
    {
        RegisterUIFromConfig(Config);
    }

    // Ԥ���ر��ΪԤ���ص�UI
    PreloadMarkedUIs();
}

bool UUIManager::RegisterUIFromConfig(const FUIConfigData& Config)
{
    if (Config.UIName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIManager::RegisterUIFromConfig - UI name is empty"));
        return false;
    }

    if (Config.WidgetClass.IsNull())
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIManager::RegisterUIFromConfig - Widget class is null for UI: %s"),
            *Config.UIName.ToString());
        return false;
    }

    // ����Ƿ���ע��
    if (UIRegistry.Contains(Config.UIName))
    {
        UE_LOG(LogTemp, Warning, TEXT("UUIManager::RegisterUIFromConfig - UI already registered: %s"),
            *Config.UIName.ToString());
        return false;
    }

    // ����Widget��
    TSubclassOf<UUserWidget> WidgetClass = LoadWidgetClass(Config.WidgetClass);
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::RegisterUIFromConfig - Failed to load widget class for UI: %s"),
            *Config.UIName.ToString());
        return false;
    }

    // ע��UI��Ϣ
    FUIInfo UIInfo;
    UIInfo.UIName = Config.UIName;
    UIInfo.WidgetClass = WidgetClass;
    UIInfo.Layer = Config.DefaultLayer;
    UIInfo.State = EUIState::Hidden;
    UIInfo.WidgetInstance = nullptr;
    UIInfo.bIsPreloaded = Config.bPreload;

    UIRegistry.Add(Config.UIName, UIInfo);

    UE_LOG(LogTemp, Log, TEXT("UUIManager::RegisterUIFromConfig - Registered UI: %s, Class: %s, Layer: %s, Preload: %s"),
        *Config.UIName.ToString(),
        *WidgetClass->GetName(),
        *UEnum::GetValueAsString(Config.DefaultLayer),
        Config.bPreload ? TEXT("Yes") : TEXT("No"));

    return true;
}

TSubclassOf<UUserWidget> UUIManager::LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr)
{
    if (SoftClassPtr.IsNull())
    {
        return nullptr;
    }

    // ͬ������Widget��
    TSubclassOf<UUserWidget> WidgetClass = SoftClassPtr.LoadSynchronous();
    if (!WidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::LoadWidgetClass - Failed to load widget class: %s"),
            *SoftClassPtr.ToString());
    }

    return WidgetClass;
}

void UUIManager::PreloadUIs(const TArray<FName>& UINames)
{
    for (const FName& UIName : UINames)
    {
        if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
        {
            if (!UIInfo->WidgetInstance && UIInfo->WidgetClass)
            {
                // ����������ʾ
                UUserWidget* Widget = CreateWidget<UUserWidget>(WorldContext, UIInfo->WidgetClass);
                if (Widget)
                {
                    UIInfo->WidgetInstance = Widget;
                    UIInfo->bIsPreloaded = true;

                    UE_LOG(LogTemp, Log, TEXT("UUIManager::PreloadUIs - Preloaded UI: %s"), *UIName.ToString());
                }
            }
        }
    }
}

void UUIManager::PreloadMarkedUIs()
{
    if (!UIConfigData)
    {
        return;
    }

    TArray<FUIConfigData> PreloadConfigs = UIConfigData->GetPreloadUIConfigs();

    for (const FUIConfigData& Config : PreloadConfigs)
    {
        if (FUIInfo* UIInfo = UIRegistry.Find(Config.UIName))
        {
            if (!UIInfo->WidgetInstance && UIInfo->WidgetClass)
            {
                // ����������ʾ
                UUserWidget* Widget = CreateWidget<UUserWidget>(WorldContext, UIInfo->WidgetClass);
                if (Widget)
                {
                    UIInfo->WidgetInstance = Widget;
                    UIInfo->bIsPreloaded = true;

                    UE_LOG(LogTemp, Log, TEXT("UUIManager::PreloadMarkedUIs - Preloaded UI: %s"),
                        *Config.UIName.ToString());
                }
            }
        }
    }
}

UUserWidget* UUIManager::ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer, UObject* Data, FName UIName)
{
    if (!WidgetClass || !WorldContext)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::ShowUI - Invalid WidgetClass or WorldContext"));
        return nullptr;
    }

    if (UIName.IsNone())
    {
        UIName = FName(*WidgetClass->GetName());
        UIName = FName(*UIName.ToString().Replace(TEXT("_C"), TEXT(""))); // �Ƴ���׺
    }

    // ����Ƿ��Ѵ���
    if (FUIInfo* ExistingUI = UIRegistry.Find(UIName))
    {
        if (ExistingUI->WidgetInstance)
        {
            ExistingUI->WidgetInstance->SetVisibility(ESlateVisibility::Visible);
            ExistingUI->State = EUIState::Visible;

            if (Data)
            {
                UUIBase* tempUIBase = Cast<UUIBase>(ExistingUI->WidgetInstance);
                if (tempUIBase)
                {
                    tempUIBase->SetData(Data);
                }
            }

            OnUIShown.Broadcast(UIName);
            return ExistingUI->WidgetInstance;
        }
    }

    // ������UI
    UUserWidget* NewWidget = CreateWidget<UUserWidget>(WorldContext, WidgetClass);
    if (!NewWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIManager::ShowUI - Failed to create widget"));
        return nullptr;
    }

    // ��ӵ��㼶
    AddToLayer(NewWidget, Layer);

    // ע��UI��Ϣ�������δע�ᣩ
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
        // ��������UI��Ϣ
        FUIInfo* UIInfo = UIRegistry.Find(UIName);
        UIInfo->WidgetInstance = NewWidget;
        UIInfo->State = EUIState::Visible;
    }

    // ��������
    if (Data)
    {
        // ������Ҫ�����UI������������
    }

    // ��ʾUI
    NewWidget->AddToViewport();

    OnUIShown.Broadcast(UIName);

    UE_LOG(LogTemp, Log, TEXT("UUIManager::ShowUI - Created and showed UI: %s"), *UIName.ToString());

    return NewWidget;
}

UUserWidget* UUIManager::ShowUIByName(FName UIName, UObject* Data)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        // ����Ѿ�Ԥ���أ�ֱ��ʹ��Ԥ���ص�ʵ��
        if (UIInfo->WidgetInstance)
        {
            UIInfo->WidgetInstance->SetVisibility(ESlateVisibility::Visible);
            UIInfo->State = EUIState::Visible;

            if (Data)
            {
                UUIBase* tempUIBase = Cast<UUIBase>(UIInfo->WidgetInstance);
                if (tempUIBase)
                {
                    tempUIBase->SetData(Data);
                }
            }

            // ��ӵ��㼶�������δ��ӣ�
            if (!UIInfo->WidgetInstance->IsInViewport())
            {
                AddToLayer(UIInfo->WidgetInstance, UIInfo->Layer);
                UIInfo->WidgetInstance->AddToViewport();
            }

            OnUIShown.Broadcast(UIName);
            return UIInfo->WidgetInstance;
        }
        else
        {
            // û��Ԥ���أ�������ʵ��
            return ShowUI(UIInfo->WidgetClass, UIInfo->Layer, Data, UIName);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("UUIManager::ShowUIByName - UI not registered: %s"), *UIName.ToString());
    return nullptr;
}

void UUIManager::HideUI(FName UIName)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            UIInfo->WidgetInstance->SetVisibility(ESlateVisibility::Hidden);
            UIInfo->State = EUIState::Hidden;
            OnUIHidden.Broadcast(UIName);
        }
    }
}

void UUIManager::CloseUI(FName UIName)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (IsValid(UIInfo->WidgetInstance))
        {
            RemoveFromLayer(UIInfo->WidgetInstance);
            UIInfo->WidgetInstance->RemoveFromParent();
            UIInfo->WidgetInstance = nullptr;
        }

        // ���UI����ͨ������ע��ģ����ע������Ƴ�
        FUIConfigData TempConfig;  // ������ʱ����
        if (!UIConfigData || !UIConfigData->GetUIConfig(UIName, TempConfig))
        {
            UIRegistry.Remove(UIName);
        }
        else
        {
            // ����ʵ����״̬
            UIInfo->WidgetInstance = nullptr;
            UIInfo->State = EUIState::Hidden;
        }

        OnUIClosed.Broadcast(UIName);
    }
}

void UUIManager::CloseAllUI()
{
    TArray<FName> UINames;
    UIRegistry.GetKeys(UINames);

    for (const FName& UIName : UINames)
    {
        CloseUI(UIName);
    }
}

UUserWidget* UUIManager::GetUI(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        return UIInfo->WidgetInstance;
    }
    return nullptr;
}

bool UUIManager::IsUIVisible(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        return UIInfo->State == EUIState::Visible && UIInfo->WidgetInstance != nullptr;
    }
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
        if (Pair.Value.WidgetInstance != nullptr && Pair.Value.State == EUIState::Visible)
        {
            ActiveUIs.Add(Pair.Key);
        }
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
        if (UIInfo->WidgetInstance)
        {
            RemoveFromLayer(UIInfo->WidgetInstance);
            AddToLayer(UIInfo->WidgetInstance, NewLayer);
        }
        UIInfo->Layer = NewLayer;
    }
}

EUIPanelLayer UUIManager::GetUILayer(FName UIName) const
{
    if (const FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        return UIInfo->Layer;
    }
    return EUIPanelLayer::None;
}

UCanvasPanel* UUIManager::GetOrCreateLayerPanel(EUIPanelLayer Layer)
{
    if (UCanvasPanel** FoundPanel = LayerPanels.Find(Layer))
    {
        return *FoundPanel;
    }
    return nullptr;
}

void UUIManager::AddToLayer(UUserWidget* Widget, EUIPanelLayer Layer)
{
    // ��ʵ��ʵ���У�������Ҫ��Widget��ӵ�ָ���㼶�Ļ�����
    UE_LOG(LogTemp, Log, TEXT("UUIManager::AddToLayer - Added widget to layer: %s"),
        *UEnum::GetValueAsString(Layer));
}

void UUIManager::RemoveFromLayer(UUserWidget* Widget)
{
    // ��ʵ��ʵ���У�������Ҫ�Ӳ㼶�������Ƴ�Widget
    UE_LOG(LogTemp, Log, TEXT("UUIManager::RemoveFromLayer - Removed widget from layer"));
}

void UUIManager::FadeInUI(FName UIName, float Duration)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            UIInfo->State = EUIState::Showing;
            HandleFadeAnimation(UIName, 1.0f, Duration);
        }
    }
}

void UUIManager::FadeOutUI(FName UIName, float Duration)
{
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            UIInfo->State = EUIState::Hiding;
            HandleFadeAnimation(UIName, 0.0f, Duration);
        }
    }
}

void UUIManager::HandleFadeAnimation(FName UIName, float TargetAlpha, float Duration)
{
    // ��ʵ��ʵ���У�������Ҫʹ��UMG�Ķ���ϵͳ
    if (FUIInfo* UIInfo = UIRegistry.Find(UIName))
    {
        if (UIInfo->WidgetInstance)
        {
            UIInfo->WidgetInstance->SetRenderOpacity(TargetAlpha);

            if (TargetAlpha >= 1.0f)
            {
                UIInfo->State = EUIState::Visible;
            }
            else if (TargetAlpha <= 0.0f)
            {
                UIInfo->State = EUIState::Hidden;
            }
        }
    }
}

void UUIManager::PrintAllUIs()
{
    UE_LOG(LogTemp, Log, TEXT("=== All Registered UIs ==="));
    for (const auto& Pair : UIRegistry)
    {
        const FUIInfo& UIInfo = Pair.Value;
        UE_LOG(LogTemp, Log, TEXT("UI: %s, Class: %s, Layer: %s, State: %s, Instance: %s, Preloaded: %s"),
            *UIInfo.UIName.ToString(),
            *UIInfo.WidgetClass->GetName(),
            *UEnum::GetValueAsString(UIInfo.Layer),
            *UEnum::GetValueAsString(UIInfo.State),
            UIInfo.WidgetInstance ? TEXT("Valid") : TEXT("Null"),
            UIInfo.bIsPreloaded ? TEXT("Yes") : TEXT("No"));
    }
    UE_LOG(LogTemp, Log, TEXT("=========================="));
}

void UUIManager::PrintLayerInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== UI Layer Information ==="));
    for (const auto& Pair : LayerPanels)
    {
        UE_LOG(LogTemp, Log, TEXT("Layer: %s, Panel: %s"),
            *UEnum::GetValueAsString(Pair.Key),
            Pair.Value ? TEXT("Valid") : TEXT("Null"));
    }
    UE_LOG(LogTemp, Log, TEXT("============================"));
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
            UE_LOG(LogTemp, Log, TEXT("  UI: %s, Class: %s, Layer: %s, Preload: %s, Desc: %s"),
                *Config.UIName.ToString(),
                *Config.WidgetClass.ToString(),
                *UEnum::GetValueAsString(Config.DefaultLayer),
                Config.bPreload ? TEXT("Yes") : TEXT("No"),
                *Config.Description);
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No config data asset set"));
    }
    UE_LOG(LogTemp, Log, TEXT("=============================="));
}

UWorld* UUIManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                return Context.World();
            }
        }
    }
    return nullptr;
}