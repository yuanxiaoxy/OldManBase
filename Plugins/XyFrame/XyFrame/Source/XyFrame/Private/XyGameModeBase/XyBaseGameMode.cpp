// Fill out your copyright notice in the Description page of Project Settings.

#include "XyGameModeBase/XyBaseGameMode.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AXyBaseGameMode::AXyBaseGameMode()
{
    WorldInitState = EWorldInitState::NotInitialized;

    // 设置默认玩家控制器类
    static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/BP_PlayerController"));
    if (PlayerControllerBPClass.Class != nullptr)
    {
        PlayerControllerClass = PlayerControllerBPClass.Class;
    }

    // 设置默认玩家Pawn类
    static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_PlayerPawn"));
    if (PlayerPawnBPClass.Class != nullptr)
    {
        DefaultPawnClass = PlayerPawnBPClass.Class;
        PlayerSpawnConfiguration.PlayerPawnClass = PlayerPawnBPClass.Class;
    }
}

void AXyBaseGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("BaseGameMode BeginPlay - Starting world initialization"));

    // 根据配置选择同步或异步初始化
    FWorldConfig Config = GetWorldConfig();
    if (Config.bAsyncInitialization)
    {
        InitializeWorldAsync();
    }
    else
    {
        InitializeWorld();
    }
}

void AXyBaseGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("BaseGameMode EndPlay - Shutting down world"));

    // 关闭世界
    ShutdownWorld();

    Super::EndPlay(EndPlayReason);
}

// ========== 世界生命周期管理 ==========

void AXyBaseGameMode::InitializeWorld()
{
    if (WorldInitState == EWorldInitState::Initializing || WorldInitState == EWorldInitState::Initialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("World is already initializing or initialized"));
        return;
    }

    WorldInitState = EWorldInitState::Initializing;
    UE_LOG(LogTemp, Log, TEXT("Starting world initialization..."));

    bool bSuccess = true;

    // 执行初始化步骤
    PreInitializeWorld();

    InitializeFrameworkSystems();
    LoadWorldResources();
    InitializeWorldState();
    InitializePlayers();

    // 如果没有发生错误，则认为初始化成功
    if (bSuccess)
    {
        PostInitializeWorld();
        WorldInitState = EWorldInitState::Initialized;
        UE_LOG(LogTemp, Log, TEXT("World initialization completed successfully"));
    }
    else
    {
        WorldInitState = EWorldInitState::Failed;
        UE_LOG(LogTemp, Error, TEXT("World initialization failed"));
    }

    // 处理初始化完成
    HandleWorldInitialized(bSuccess);
}

void AXyBaseGameMode::InitializeWorldAsync()
{
    if (WorldInitState != EWorldInitState::NotInitialized)
    {
        return;
    }

    FWorldConfig Config = GetWorldConfig();

    if (Config.InitializationDelay > 0.0f)
    {
        // 延迟初始化 - 使用成员函数绑定
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            AsyncInitTimerId = MonoMgr->SetTimeout(Config.InitializationDelay, this, &AXyBaseGameMode::InitializeWorld);
        }
    }
    else
    {
        // 立即初始化（下一帧）
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
                this->InitializeWorld();
                });
        }
    }
}

void AXyBaseGameMode::ShutdownWorld()
{
    UE_LOG(LogTemp, Log, TEXT("Shutting down world..."));

    // 取消异步初始化计时器
    if (!AsyncInitTimerId.IsEmpty())
    {
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            MonoMgr->ClearTimer(AsyncInitTimerId);
        }
    }

    // 执行关闭步骤
    PreShutdownWorld();
    SaveWorldState();
    CleanupWorldResources();
    PostShutdownWorld();

    // 广播事件
    OnWorldShutdown.Broadcast();

    UE_LOG(LogTemp, Log, TEXT("World shutdown completed"));
}

void AXyBaseGameMode::RestartWorld()
{
    UE_LOG(LogTemp, Log, TEXT("Restarting world..."));

    ShutdownWorld();
    WorldInitState = EWorldInitState::NotInitialized;
    InitializeWorld();
}

// ========== 玩家管理默认实现 ==========

bool AXyBaseGameMode::ShouldSpawnPlayer_Implementation() const
{
    return PlayerSpawnConfiguration.bShouldSpawnPlayer;
}

FPlayerSpawnConfig AXyBaseGameMode::GetPlayerSpawnConfig_Implementation() const
{
    return PlayerSpawnConfiguration;
}

APawn* AXyBaseGameMode::SpawnPlayer_Implementation(AController* NewPlayer)
{
    if (!ShouldSpawnPlayer())
    {
        UE_LOG(LogTemp, Log, TEXT("Player spawning disabled by configuration"));
        return nullptr;
    }

    return SpawnDefaultPlayer(NewPlayer);
}

FTransform AXyBaseGameMode::GetPlayerSpawnTransform_Implementation(AController* PlayerController)
{
    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();

    if (Config.bUseRandomSpawn && Config.PossibleSpawnPoints.Num() > 0)
    {
        return GetRandomSpawnTransform();
    }

    return Config.SpawnTransform;
}

// ========== 世界配置默认实现 ==========

FWorldConfig AXyBaseGameMode::GetWorldConfig_Implementation() const
{
    return WorldConfiguration;
}

void AXyBaseGameMode::SetWorldConfig(const FWorldConfig& NewConfig)
{
    WorldConfiguration = NewConfig;
}

// ========== 框架集成 ==========

UResourceManager* AXyBaseGameMode::GetResourceManager() const
{
    return UResourceManager::GetResourceManager();
}

UMonoManager* AXyBaseGameMode::GetMonoManager() const
{
    return UMonoManager::GetMonoManager();
}

USaveGameTool* AXyBaseGameMode::GetSaveGameTool() const
{
    return USaveGameTool::GetSaveGameTool();
}

// ========== 初始化步骤默认实现 ==========

void AXyBaseGameMode::PreInitializeWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Pre-initializing world..."));
    // 基类实现为空，子类可以重写
}

void AXyBaseGameMode::InitializeFrameworkSystems_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing framework systems..."));

    // 确保框架单例已初始化
    UResourceManager* ResourceMgr = GetResourceManager();
    UMonoManager* MonoMgr = GetMonoManager();
    USaveGameTool* SaveTool = GetSaveGameTool();

    if (ResourceMgr) ResourceMgr->InitializeResourceManager();
    if (MonoMgr) MonoMgr->InitializeMonoManager();
    if (SaveTool) SaveTool->InitializeSaveTool();

    UE_LOG(LogTemp, Log, TEXT("Framework systems initialized"));
}

void AXyBaseGameMode::LoadWorldResources_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Loading world resources..."));
    // 基类实现为空，子类可以重写来加载特定资源
}

void AXyBaseGameMode::InitializeWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing world state..."));

    FWorldConfig Config = GetWorldConfig();

    // 如果需要从存档加载
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
            // 尝试加载世界数据
            FWorldSaveData WorldData;
            if (SaveTool->LoadWorldDataStruct(WorldData, Config.SaveSlotName))
            {
                UE_LOG(LogTemp, Log, TEXT("World state loaded from save: %s"), *Config.SaveSlotName);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Failed to load world state from save: %s"), *Config.SaveSlotName);
            }
        }
    }
}

void AXyBaseGameMode::InitializePlayers_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing players..."));
    // 基类实现为空，子类可以重写来初始化玩家相关逻辑
}

void AXyBaseGameMode::PostInitializeWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-initializing world..."));
    // 基类实现为空，子类可以重写
}

// ========== 关闭步骤默认实现 ==========

void AXyBaseGameMode::PreShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Pre-shutting down world..."));
    // 基类实现为空，子类可以重写
}

void AXyBaseGameMode::SaveWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Saving world state..."));

    FWorldConfig Config = GetWorldConfig();

    // 如果配置了存档，保存世界状态
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
            FWorldSaveData WorldData;
            // 子类可以重写来填充世界数据
            if (SaveTool->SaveWorldDataStruct(WorldData, Config.SaveSlotName))
            {
                UE_LOG(LogTemp, Log, TEXT("World state saved: %s"), *Config.SaveSlotName);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to save world state: %s"), *Config.SaveSlotName);
            }
        }
    }
}

void AXyBaseGameMode::CleanupWorldResources_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Cleaning up world resources..."));

    // 清理资源管理器缓存（可选）
    UResourceManager* ResourceMgr = GetResourceManager();
    if (ResourceMgr)
    {
        ResourceMgr->ClearCache();
    }

    //清理所有单例
    USingletonManager::GetInstance()->DestroyAllSingletons();
}

void AXyBaseGameMode::PostShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-shutting down world..."));
    // 基类实现为空，子类可以重写
}

// ========== 其他方法实现 ==========

void AXyBaseGameMode::InitializeWorldWithCallback(const FOnWorldInitCallback& Callback)
{
    if (Callback.IsBound())
    {
        InitCallbacks.Add(Callback);
    }

    if (WorldInitState == EWorldInitState::Initialized)
    {
        // 如果已经初始化，立即执行回调
        Callback.Execute(true);
    }
    else if (WorldInitState == EWorldInitState::NotInitialized)
    {
        // 如果未初始化，开始初始化
        InitializeWorld();
    }
    // 如果正在初始化，回调会在初始化完成后执行
}

void AXyBaseGameMode::HandleWorldInitialized(bool bSuccess)
{
    // 广播事件
    OnWorldInitialized.Broadcast(bSuccess);

    // 执行回调
    for (const FOnWorldInitCallback& Callback : InitCallbacks)
    {
        if (Callback.IsBound())
        {
            Callback.Execute(bSuccess);
        }
    }
    InitCallbacks.Empty();
}

void AXyBaseGameMode::ExecuteAsyncInitialization()
{
    // 这个方法现在不需要了，因为我们在InitializeWorldAsync中直接处理了
    // 保留这个空实现以避免链接错误
}

APawn* AXyBaseGameMode::SpawnDefaultPlayer(AController* NewPlayer)
{
    if (!NewPlayer)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn player: Invalid controller"));
        return nullptr;
    }

    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();
    FTransform SpawnTransform = GetPlayerSpawnTransform(NewPlayer);

    // 确定要生成的Pawn类
    TSubclassOf<APawn> PawnClass = Config.PlayerPawnClass;
    if (!PawnClass)
    {
        PawnClass = DefaultPawnClass;
    }

    if (!PawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn player: No valid pawn class specified"));
        return nullptr;
    }

    // 生成玩家Pawn
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = NewPlayer;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnParams);

    if (SpawnedPawn)
    {
        // 设置玩家控制器
        NewPlayer->Possess(SpawnedPawn);
        UE_LOG(LogTemp, Log, TEXT("Player spawned successfully: %s"), *SpawnedPawn->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn player pawn"));
    }

    return SpawnedPawn;
}

FTransform AXyBaseGameMode::GetRandomSpawnTransform() const
{
    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();

    if (Config.PossibleSpawnPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No spawn points available, using default transform"));
        return Config.SpawnTransform;
    }

    int32 RandomIndex = FMath::RandRange(0, Config.PossibleSpawnPoints.Num() - 1);
    return Config.PossibleSpawnPoints[RandomIndex];
}