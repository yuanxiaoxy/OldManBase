// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/OldManGameMode.h"
#include "UIManager/UIManager.h"
#include "EffectManager/EffectManager.h"
#include "AudioManager/AudioManager.h"
#include "UIManager/UIConfigDataAsset.h"
#include "ResourceManager/ResourceManager.h"
#include "SavePoint/OldManSavePointManager.h"
#include "SceneManager/LoadSceneManager.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Kismet/GameplayStatics.h"
#include "TaskSystem/MissionManager.h"

AOldManGameMode::AOldManGameMode()
{
    OnWorldInitialized.AddDynamic(this, &AOldManGameMode::OnMyWorldInitialized);
}

// ===== 新增：在组件初始化后立即执行，提前加载音频数据表 =====
void AOldManGameMode::PreInitializeComponents()
{
    Super::PreInitializeComponents();

    // 提前初始化音频管理器（如果数据表已赋值）
    if (AudioConfig)
    {
        UAudioManager::GetInstance()->Initialize(AudioConfig);
        UE_LOG(LogTemp, Log, TEXT("AudioManager initialized in PreInitializeComponents"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioConfig is null in PreInitializeComponents, audio will be initialized later."));
    }
}

void AOldManGameMode::BeginPlay()
{
    Super::BeginPlay();
}

APawn* AOldManGameMode::SpawnPlayer_Implementation(AController* NewPlayer)
{
    if (IsStartWorld && !bShouldSpawnPlayerLater)
    {
        // 保存玩家控制器，等待UI操作后再生成玩家
        PendingPlayerController = NewPlayer;
        return nullptr;
    }

    return Super::SpawnPlayer_Implementation(NewPlayer);
}

void AOldManGameMode::LoadWorldResources_Implementation()
{
    Super::LoadWorldResources_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Loading my specific world resources..."));
}

void AOldManGameMode::InitializeWorldState_Implementation()
{
    Super::InitializeWorldState_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Initializing my specific world state..."));

    // 不在BeginPlay中直接显示UI，等待世界初始化完成
    UIManager = UUIManager::GetInstance();
    // 初始化UI, Effect系统等
    UUIConfigDataAsset* UIConfigData = Cast<UUIConfigDataAsset>(UIConfig);
    if (UIConfigData && UIManager)
    {
        UIManager->InitializeUIManager(UIConfigData);
        // 监听UI配置加载完成事件
        UIManager->OnUIConfigLoaded.AddDynamic(this, &AOldManGameMode::OnUIInitialized);
    }

    UDataTable* EffectConfigData = Cast<UDataTable>(EffectConfig);
    if (EffectConfigData)
    {
        UEffectManager::GetInstance()->SetEffectDataTable(EffectConfigData);
        UEffectManager::GetInstance()->StartUpdating(0.033f);
    }

    // 音频管理器已在 PreInitializeComponents 中初始化，这里无需重复
    // 但可以检查是否已经成功初始化，或者处理可能为空的备用方案
    UDataTable* AudioConfigData = Cast<UDataTable>(AudioConfig);
    if (AudioConfigData)
    {
        // 如果 PreInitializeComponents 中未初始化（例如 AudioConfig 为空），这里再尝试一次
        if (!UAudioManager::GetInstance()->IsInitialized())
        {
            UAudioManager::GetInstance()->Initialize(AudioConfigData);
        }
    }

    UDataTable* TaskConfigData = Cast<UDataTable>(TaskConfig);
    if (TaskConfigData)
    {
        UMissionManager::GetInstance()->LoadTaskTable(TaskConfigData);
    }

    UOldManSavePointManager::GetInstance();
    ULoadSceneManager::GetInstance()->InitializeSceneManager();
}

void AOldManGameMode::InitializePlayers_Implementation()
{
    Super::InitializePlayers_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Initializing my specific players..."));

    // 不再在这里显示UI，等待世界初始化完成
}

void AOldManGameMode::OnMyWorldInitialized(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("My world initialized successfully!"));

        if (IsStartWorld)
        {
            // 等待一帧确保UI管理器完全初始化
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUFunction(this, FName("ShowStartUI"));
            GetWorld()->GetTimerManager().SetTimer(UIInitTimerHandle, TimerDelegate, 0.1f, false);
        }
        else
        {
            // 非起始世界，正常生成玩家
            bShouldSpawnPlayerLater = true;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("My world initialization failed!"));
    }
}

void AOldManGameMode::OnUIInitialized(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("UI config loaded successfully!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UI config loading failed!"));
    }
}

void AOldManGameMode::ShowStartUI()
{
    if (!UIManager || bIsUIShown)
        return;

    // 确保UI管理器已初始化
    if (!UIManager->DoesUIExist("StartPanel"))
    {
        UE_LOG(LogTemp, Error, TEXT("StartPanel not found in UIManager registry!"));

        // 再次尝试，稍后重试
        if (UIInitTimerHandle.IsValid())
        {
            GetWorld()->GetTimerManager().ClearTimer(UIInitTimerHandle);
        }

        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUFunction(this, FName("ShowStartUI"));
        GetWorld()->GetTimerManager().SetTimer(UIInitTimerHandle, TimerDelegate, 0.5f, false);
        return;
    }

    // 显示开始UI
    UUserWidget* StartPanel = UIManager->ShowUIByName("StartPanel");
    if (StartPanel)
    {
        UE_LOG(LogTemp, Log, TEXT("StartPanel created successfully: %s"), *StartPanel->GetName());
        bIsUIShown = true;

        // 设置输入模式为UI（显示鼠标）
        SetupInputModeForUI();
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create StartPanel!"));
    }
}

void AOldManGameMode::SetupInputModeForUI()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        // 显示鼠标光标
        PlayerController->bShowMouseCursor = true;
        PlayerController->bEnableClickEvents = true;
        PlayerController->bEnableMouseOverEvents = true;

        // 设置输入模式为UI
        FInputModeUIOnly InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PlayerController->SetInputMode(InputMode);

        UE_LOG(LogTemp, Log, TEXT("Input mode set to UI (mouse visible)"));
    }
}

void AOldManGameMode::SetupInputModeForGame()
{
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        // 隐藏鼠标光标
        PlayerController->bShowMouseCursor = false;

        // 设置输入模式为游戏
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);

        UE_LOG(LogTemp, Log, TEXT("Input mode set to Game (mouse hidden)"));
    }
}