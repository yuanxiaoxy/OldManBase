// Fill out your copyright notice in the Description page of Project Settings.

#include "XyGameModeBase/XyBaseGameMode.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AXyBaseGameMode::AXyBaseGameMode()
{
    WorldInitState = EWorldInitState::NotInitialized;

    // ����Ĭ����ҿ�������
    static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/BP_PlayerController"));
    if (PlayerControllerBPClass.Class != nullptr)
    {
        PlayerControllerClass = PlayerControllerBPClass.Class;
    }

    // ����Ĭ�����Pawn��
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

    // ��������ѡ��ͬ�����첽��ʼ��
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

    // �ر�����
    ShutdownWorld();

    Super::EndPlay(EndPlayReason);
}

// ========== �����������ڹ��� ==========

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

    // ִ�г�ʼ������
    PreInitializeWorld();

    InitializeFrameworkSystems();
    LoadWorldResources();
    InitializeWorldState();
    InitializePlayers();

    // ���û�з�����������Ϊ��ʼ���ɹ�
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

    // �����ʼ�����
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
        // �ӳٳ�ʼ�� - ʹ�ó�Ա������
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            AsyncInitTimerId = MonoMgr->SetTimeout(Config.InitializationDelay, this, &AXyBaseGameMode::InitializeWorld);
        }
    }
    else
    {
        // ������ʼ������һ֡��
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

    // ȡ���첽��ʼ����ʱ��
    if (!AsyncInitTimerId.IsEmpty())
    {
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            MonoMgr->ClearTimer(AsyncInitTimerId);
        }
    }

    // ִ�йرղ���
    PreShutdownWorld();
    SaveWorldState();
    CleanupWorldResources();
    PostShutdownWorld();

    // �㲥�¼�
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

// ========== ��ҹ���Ĭ��ʵ�� ==========

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

// ========== ��������Ĭ��ʵ�� ==========

FWorldConfig AXyBaseGameMode::GetWorldConfig_Implementation() const
{
    return WorldConfiguration;
}

void AXyBaseGameMode::SetWorldConfig(const FWorldConfig& NewConfig)
{
    WorldConfiguration = NewConfig;
}

// ========== ��ܼ��� ==========

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

// ========== ��ʼ������Ĭ��ʵ�� ==========

void AXyBaseGameMode::PreInitializeWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Pre-initializing world..."));
    // ����ʵ��Ϊ�գ����������д
}

void AXyBaseGameMode::InitializeFrameworkSystems_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing framework systems..."));

    // ȷ����ܵ����ѳ�ʼ��
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
    // ����ʵ��Ϊ�գ����������д�������ض���Դ
}

void AXyBaseGameMode::InitializeWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing world state..."));

    FWorldConfig Config = GetWorldConfig();

    // �����Ҫ�Ӵ浵����
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
            // ���Լ�����������
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
    // ����ʵ��Ϊ�գ����������д����ʼ���������߼�
}

void AXyBaseGameMode::PostInitializeWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-initializing world..."));
    // ����ʵ��Ϊ�գ����������д
}

// ========== �رղ���Ĭ��ʵ�� ==========

void AXyBaseGameMode::PreShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Pre-shutting down world..."));
    // ����ʵ��Ϊ�գ����������д
}

void AXyBaseGameMode::SaveWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Saving world state..."));

    FWorldConfig Config = GetWorldConfig();

    // ��������˴浵����������״̬
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
            FWorldSaveData WorldData;
            // ���������д�������������
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

    // ������Դ���������棨��ѡ��
    UResourceManager* ResourceMgr = GetResourceManager();
    if (ResourceMgr)
    {
        ResourceMgr->ClearCache();
    }

    //�������е���
    USingletonManager::GetInstance()->DestroyAllSingletons();
}

void AXyBaseGameMode::PostShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-shutting down world..."));
    // ����ʵ��Ϊ�գ����������д
}

// ========== ��������ʵ�� ==========

void AXyBaseGameMode::InitializeWorldWithCallback(const FOnWorldInitCallback& Callback)
{
    if (Callback.IsBound())
    {
        InitCallbacks.Add(Callback);
    }

    if (WorldInitState == EWorldInitState::Initialized)
    {
        // ����Ѿ���ʼ��������ִ�лص�
        Callback.Execute(true);
    }
    else if (WorldInitState == EWorldInitState::NotInitialized)
    {
        // ���δ��ʼ������ʼ��ʼ��
        InitializeWorld();
    }
    // ������ڳ�ʼ�����ص����ڳ�ʼ����ɺ�ִ��
}

void AXyBaseGameMode::HandleWorldInitialized(bool bSuccess)
{
    // �㲥�¼�
    OnWorldInitialized.Broadcast(bSuccess);

    // ִ�лص�
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
    // ����������ڲ���Ҫ�ˣ���Ϊ������InitializeWorldAsync��ֱ�Ӵ�����
    // ���������ʵ���Ա������Ӵ���
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

    // ȷ��Ҫ���ɵ�Pawn��
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

    // �������Pawn
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = NewPlayer;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnParams);

    if (SpawnedPawn)
    {
        // ������ҿ�����
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