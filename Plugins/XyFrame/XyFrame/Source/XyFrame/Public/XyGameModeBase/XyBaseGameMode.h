// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SingletonBase/SingletonBase.h"
#include "ResourceManager/ResourceManager.h"
#include "MonoManager/MonoManager.h"
#include "SaveManager/SaveGameTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "XyBaseGameMode.generated.h"

// 前向声明
class APlayerController;
class AController;
class APawn;

// 世界初始化状态
UENUM(BlueprintType)
enum class EWorldInitState : uint8
{
    NotInitialized UMETA(DisplayName = "Not Initialized"),
    Initializing   UMETA(DisplayName = "Initializing"),
    Initialized    UMETA(DisplayName = "Initialized"),
    Failed         UMETA(DisplayName = "Failed")
};

// 玩家生成配置
USTRUCT(BlueprintType)
struct FPlayerSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    bool bShouldSpawnPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    TSubclassOf<APawn> PlayerPawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    FTransform SpawnTransform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    bool bUseRandomSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
    TArray<FTransform> PossibleSpawnPoints;

    // 相机出生点配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Spawn")
    bool bUseCameraSpawn = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Spawn")
    FVector CameraSpawnOffset = FVector(0.0f, 0.0f, 0.0f);

    FPlayerSpawnConfig()
        : bShouldSpawnPlayer(true)
        , bUseRandomSpawn(false)
    {
    }
};

// 世界配置
USTRUCT(BlueprintType)
struct FWorldConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FString WorldName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FString WorldDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bLoadFromSave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    FString SaveSlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    bool bAsyncInitialization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float InitializationDelay;

    FWorldConfig()
        : bLoadFromSave(false)
        , bAsyncInitialization(true)
        , InitializationDelay(0.0f)
    {
    }
};

// 委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldInitialized, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldShutdown);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWorldInitCallback, bool, bSuccess);

UCLASS(Abstract, Blueprintable, BlueprintType)
class XYFRAME_API AXyBaseGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AXyBaseGameMode();

protected:
    virtual void StartPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 世界生命周期管理 ==========
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorld();

    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorldAsync();

    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void ShutdownWorld();

    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void RestartWorld();

    // ========== 玩家管理 ==========
    virtual void RestartPlayer(AController* NewPlayer) override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    bool ShouldSpawnPlayer() const;
    virtual bool ShouldSpawnPlayer_Implementation() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FPlayerSpawnConfig GetPlayerSpawnConfig() const;
    virtual FPlayerSpawnConfig GetPlayerSpawnConfig_Implementation() const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    APawn* SpawnPlayer(AController* NewPlayer);
    virtual APawn* SpawnPlayer_Implementation(AController* NewPlayer);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FTransform GetPlayerSpawnTransform(AController* PlayerController);
    virtual FTransform GetPlayerSpawnTransform_Implementation(AController* PlayerController);

    UFUNCTION(BlueprintCallable, Category = "Player Spawn")
    void SpawnPlayerAtCamera(AController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    FTransform GetCameraTransform() const;

    // ========== 世界配置 ==========
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Config")
    FWorldConfig GetWorldConfig() const;
    virtual FWorldConfig GetWorldConfig_Implementation() const;

    UFUNCTION(BlueprintCallable, Category = "GameMode|Config")
    void SetWorldConfig(const FWorldConfig& NewConfig);

    // ========== 框架集成 ==========
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UResourceManager* GetResourceManager() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UMonoManager* GetMonoManager() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    USaveGameTool* GetSaveGameTool() const;

    // ========== 初始化步骤 ==========
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PreInitializeWorld();
    virtual void PreInitializeWorld_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeFrameworkSystems();
    virtual void InitializeFrameworkSystems_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void LoadWorldResources();
    virtual void LoadWorldResources_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeWorldState();
    virtual void InitializeWorldState_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializePlayers();
    virtual void InitializePlayers_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PostInitializeWorld();
    virtual void PostInitializeWorld_Implementation();

    // ========== 关闭步骤 ==========
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void PreShutdownWorld();
    virtual void PreShutdownWorld_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void SaveWorldState();
    virtual void SaveWorldState_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void CleanupWorldResources();
    virtual void CleanupWorldResources_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void PostShutdownWorld();
    virtual void PostShutdownWorld_Implementation();

    // ========== 事件委托 ==========
    UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
    FOnWorldInitialized OnWorldInitialized;

    UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
    FOnWorldShutdown OnWorldShutdown;

    // ========== 带回调的异步初始化 ==========
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    void InitializeWorldWithCallback(const FOnWorldInitCallback& Callback);

    // ========== 状态查询 ==========
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    EWorldInitState GetWorldInitState() const { return WorldInitState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    bool IsWorldInitialized() const { return WorldInitState == EWorldInitState::Initialized; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    bool IsWorldInitializing() const { return WorldInitState == EWorldInitState::Initializing; }

protected:
    void HandleWorldInitialized(bool bSuccess);
    void ExecuteAsyncInitialization();
    FTransform GetRandomSpawnTransform() const;
    APawn* SpawnDefaultPlayer(AController* NewPlayer);

#if WITH_EDITOR
    FTransform GetEditorViewportCameraTransform() const;
#endif

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State", meta = (AllowPrivateAccess = "true"))
    EWorldInitState WorldInitState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Config", meta = (AllowPrivateAccess = "true"))
    FWorldConfig WorldConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Player", meta = (AllowPrivateAccess = "true"))
    FPlayerSpawnConfig PlayerSpawnConfiguration;

    TArray<FOnWorldInitCallback> InitCallbacks;
    FString AsyncInitTimerId;
};