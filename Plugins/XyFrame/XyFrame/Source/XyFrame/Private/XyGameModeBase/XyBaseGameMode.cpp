// Fill out your copyright notice in the Description page of Project Settings.

#include "XyGameModeBase/XyBaseGameMode.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Math/UnrealMathUtility.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "LevelEditorViewport.h"
#include "EditorViewportClient.h"
#endif

AXyBaseGameMode::AXyBaseGameMode()
{
    WorldInitState = EWorldInitState::NotInitialized;
}

void AXyBaseGameMode::StartPlay()
{
    FWorldConfig Config = GetWorldConfig();
    if (Config.bAsyncInitialization)
    {
        InitializeWorldAsync();
    }
    else
    {
        InitializeWorld();
    }

    Super::StartPlay();

    UE_LOG(LogTemp, Log, TEXT("BaseGameMode StartPlay - Starting world initialization"));
}

void AXyBaseGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("BaseGameMode EndPlay - Shutting down world"));
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

    PreInitializeWorld();
    InitializeFrameworkSystems();
    LoadWorldResources();
    InitializeWorldState();
    InitializePlayers();

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

    HandleWorldInitialized(bSuccess);
}

void AXyBaseGameMode::InitializeWorldAsync()
{
    if (WorldInitState != EWorldInitState::NotInitialized)
        return;

    FWorldConfig Config = GetWorldConfig();

    if (Config.InitializationDelay > 0.0f)
    {
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            AsyncInitTimerId = MonoMgr->SetTimeout(Config.InitializationDelay, this, &AXyBaseGameMode::InitializeWorld);
        }
    }
    else
    {
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

    if (!AsyncInitTimerId.IsEmpty())
    {
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr) MonoMgr->ClearTimer(AsyncInitTimerId);
    }

    PreShutdownWorld();
    SaveWorldState();
    CleanupWorldResources();
    PostShutdownWorld();

    OnWorldShutdown.Broadcast();
    USingletonManager::GetInstance()->Shutdown();

    UE_LOG(LogTemp, Log, TEXT("World shutdown completed"));
}

void AXyBaseGameMode::RestartWorld()
{
    UE_LOG(LogTemp, Log, TEXT("Restarting world..."));
    ShutdownWorld();
    WorldInitState = EWorldInitState::NotInitialized;
    InitializeWorld();
}

// ========== 玩家管理 ==========
void AXyBaseGameMode::RestartPlayer(AController* NewPlayer)
{
    if (NewPlayer == nullptr || NewPlayer->IsPendingKillPending())
    {
        UE_LOG(LogTemp, Warning, TEXT("RestartPlayer: Invalid or pending kill controller"));
        return;
    }

    if (!ShouldSpawnPlayer())
    {
        UE_LOG(LogTemp, Log, TEXT("Player spawning disabled by configuration"));
        return;
    }

    AActor* StartSpot = FindPlayerStart(NewPlayer);
    if (StartSpot == nullptr && NewPlayer->StartSpot != nullptr)
    {
        StartSpot = NewPlayer->StartSpot.Get();
        UE_LOG(LogGameMode, Warning, TEXT("RestartPlayer: Player start not found, using last start spot"));
    }

    if (APawn* Pawn = SpawnPlayer(NewPlayer))
    {
        InitStartSpot(StartSpot, NewPlayer);
        FinishRestartPlayer(NewPlayer, Pawn->GetActorRotation());
    }
    else
    {
        FailedToRestartPlayer(NewPlayer);
    }
}

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
        UE_LOG(LogTemp, Log, TEXT("SpawnPlayer: Spawning disabled by ShouldSpawnPlayer"));
        return nullptr;
    }

    if (NewPlayer == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnPlayer: Invalid controller"));
        return nullptr;
    }

    return SpawnDefaultPlayer(NewPlayer);
}

FTransform AXyBaseGameMode::GetPlayerSpawnTransform_Implementation(AController* PlayerController)
{
    // 2. 未找到出生点时，继续使用原有逻辑
    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();

    if (Config.bUseCameraSpawn)
    {
        FTransform CameraTransform = GetCameraTransform();
        // 检查相机变换是否有效（位置非零 或 旋转不是单位四元数）
        if (CameraTransform.GetLocation().IsNearlyZero() && CameraTransform.GetRotation().IsIdentity())
        {
            UE_LOG(LogTemp, Warning, TEXT("Camera spawn transform is invalid, falling back to default spawn transform"));
            return Config.SpawnTransform;
        }

        CameraTransform.AddToTranslation(Config.CameraSpawnOffset);
        UE_LOG(LogTemp, Log, TEXT("Using camera spawn position: %s"), *CameraTransform.GetLocation().ToString());
        return CameraTransform;
    }

    if (Config.bUseRandomSpawn && Config.PossibleSpawnPoints.Num() > 0)
    {
        return GetRandomSpawnTransform();
    }

    // 1. 优先使用关卡中的出生点（PlayerStart）
    AActor* StartSpot = FindPlayerStart(PlayerController);
    if (StartSpot)
    {
        UE_LOG(LogTemp, Log, TEXT("Using PlayerStart spawn transform: %s"), *StartSpot->GetActorLocation().ToString());
        return StartSpot->GetActorTransform();
    }

    return Config.SpawnTransform;
}

// ========== 相机视角生成玩家功能 ==========
FTransform AXyBaseGameMode::GetCameraTransform() const
{
#if WITH_EDITOR
    FTransform EditorCamera = GetEditorViewportCameraTransform();
    // 编辑器相机有效：位置非零 或 旋转不是单位四元数（即不是默认无效值）
    if (!EditorCamera.GetLocation().IsNearlyZero() || !EditorCamera.GetRotation().IsIdentity())
    {
        return EditorCamera;
    }
#endif

    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
    if (CameraManager)
    {
        return FTransform(CameraManager->GetCameraRotation(), CameraManager->GetCameraLocation());
    }

    UE_LOG(LogTemp, Warning, TEXT("GetCameraTransform: No valid camera found, returning world origin"));
    return FTransform(FVector::ZeroVector);
}

#if WITH_EDITOR
FTransform AXyBaseGameMode::GetEditorViewportCameraTransform() const
{
    // 安全获取编辑器透视视口相机
    if (!GEditor)
    {
        return FTransform::Identity;
    }

    // 遍历所有编辑器视口客户端，寻找第一个透视视口
    const TArray<FEditorViewportClient*>& AllViewportClients = GEditor->GetAllViewportClients();
    for (FEditorViewportClient* Client : AllViewportClients)
    {
        if (Client && Client->IsPerspective())
        {
            return FTransform(Client->GetViewRotation(), Client->GetViewLocation());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("GetEditorViewportCameraTransform: No valid editor viewport found"));
    return FTransform::Identity;
}
#endif

void AXyBaseGameMode::SpawnPlayerAtCamera(AController* PlayerController)
{
    if (!PlayerController)
    {
        PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    }

    if (PlayerController == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnPlayerAtCamera: No valid PlayerController found"));
        return;
    }

    FTransform CameraTransform = GetCameraTransform();
    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();
    CameraTransform.AddToTranslation(Config.CameraSpawnOffset);

    if (APawn* NewPawn = SpawnPlayer(PlayerController))
    {
        NewPawn->SetActorTransform(CameraTransform);
        UE_LOG(LogTemp, Log, TEXT("Player spawned at camera position: %s"), *CameraTransform.GetLocation().ToString());
    }
}

// ========== 世界配置 ==========
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
}

void AXyBaseGameMode::InitializeFrameworkSystems_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing framework systems..."));

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
}

void AXyBaseGameMode::InitializeWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing world state..."));
    FWorldConfig Config = GetWorldConfig();
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
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
}

void AXyBaseGameMode::PostInitializeWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-initializing world..."));
}

// ========== 关闭步骤默认实现 ==========
void AXyBaseGameMode::PreShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Pre-shutting down world..."));
}

void AXyBaseGameMode::SaveWorldState_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Saving world state..."));
    FWorldConfig Config = GetWorldConfig();
    if (Config.bLoadFromSave && !Config.SaveSlotName.IsEmpty())
    {
        USaveGameTool* SaveTool = GetSaveGameTool();
        if (SaveTool)
        {
            FWorldSaveData WorldData;
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
    UResourceManager* ResourceMgr = GetResourceManager();
    if (ResourceMgr) ResourceMgr->ClearCache();
    USingletonManager::GetInstance()->DestroyAllSingletons();
}

void AXyBaseGameMode::PostShutdownWorld_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Post-shutting down world..."));
}

// ========== 其他方法 ==========
void AXyBaseGameMode::InitializeWorldWithCallback(const FOnWorldInitCallback& Callback)
{
    if (Callback.IsBound())
        InitCallbacks.Add(Callback);

    if (WorldInitState == EWorldInitState::Initialized)
        Callback.Execute(true);
    else if (WorldInitState == EWorldInitState::NotInitialized)
        InitializeWorld();
}

void AXyBaseGameMode::HandleWorldInitialized(bool bSuccess)
{
    OnWorldInitialized.Broadcast(bSuccess);
    for (const FOnWorldInitCallback& Callback : InitCallbacks)
    {
        if (Callback.IsBound())
        {
            Callback.Execute(bSuccess);
        }
    }
    InitCallbacks.Empty();
}

void AXyBaseGameMode::ExecuteAsyncInitialization() {}

APawn* AXyBaseGameMode::SpawnDefaultPlayer(AController* NewPlayer)
{
    if (NewPlayer == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnDefaultPlayer: Invalid controller"));
        return nullptr;
    }

    FPlayerSpawnConfig Config = GetPlayerSpawnConfig();
    FTransform SpawnTransform = GetPlayerSpawnTransform(NewPlayer);

    TSubclassOf<APawn> PawnClass = Config.PlayerPawnClass;
    if (!PawnClass)
    {
        PawnClass = DefaultPawnClass;
    }

    if (!PawnClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnDefaultPlayer: No valid pawn class specified"));
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = NewPlayer;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnParams.bNoFail = true;
    SpawnParams.bAllowDuringConstructionScript = false;

    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnParams);
    if (SpawnedPawn)
    {
        if (NewPlayer->GetPawn() == nullptr)
        {
            NewPlayer->Possess(SpawnedPawn);
            UE_LOG(LogTemp, Log, TEXT("Player spawned and possessed: %s"), *SpawnedPawn->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Controller already possessing a pawn, skipping possess"));
        }

        APlayerController* PC = Cast<APlayerController>(NewPlayer);
        if (PC)
        {
            PC->bShowMouseCursor = false;
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
            PC->SetShowMouseCursor(false);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnDefaultPlayer: Failed to spawn pawn"));
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