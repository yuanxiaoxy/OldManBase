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

// 世界初始化完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldInitialized, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldShutdown);

// 蓝图回调委托
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnWorldInitCallback, bool, bSuccess);

UCLASS(Abstract, Blueprintable, BlueprintType)
class XYFRAME_API AXyBaseGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AXyBaseGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ========== 世界生命周期管理 ==========

    // 初始化世界（主入口点）
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorld();

    // 异步初始化世界
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorldAsync();

    // 关闭世界
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void ShutdownWorld();

    // 重启世界
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void RestartWorld();

    // ========== 玩家管理 ==========

    // 是否应该生成玩家（可重写）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    bool ShouldSpawnPlayer() const;
    virtual bool ShouldSpawnPlayer_Implementation() const;

    // 获取玩家生成配置（可重写）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FPlayerSpawnConfig GetPlayerSpawnConfig() const;
    virtual FPlayerSpawnConfig GetPlayerSpawnConfig_Implementation() const;

    // 生成玩家（可重写）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    APawn* SpawnPlayer(AController* NewPlayer);
    virtual APawn* SpawnPlayer_Implementation(AController* NewPlayer);

    // 自定义玩家生成位置（可重写）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FTransform GetPlayerSpawnTransform(AController* PlayerController);
    virtual FTransform GetPlayerSpawnTransform_Implementation(AController* PlayerController);

    // ========== 世界配置 ==========

    // 获取世界配置（可重写）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Config")
    FWorldConfig GetWorldConfig() const;
    virtual FWorldConfig GetWorldConfig_Implementation() const;

    // 设置世界配置
    UFUNCTION(BlueprintCallable, Category = "GameMode|Config")
    void SetWorldConfig(const FWorldConfig& NewConfig);

    // ========== 框架集成 ==========

    // 获取资源管理器
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UResourceManager* GetResourceManager() const;

    // 获取计时器管理器
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UMonoManager* GetMonoManager() const;

    // 获取存档工具
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    USaveGameTool* GetSaveGameTool() const;

    // ========== 初始化步骤（可重写） ==========

    // 预初始化（在主要初始化之前）
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PreInitializeWorld();
    virtual void PreInitializeWorld_Implementation();

    // 初始化框架系统
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeFrameworkSystems();
    virtual void InitializeFrameworkSystems_Implementation();

    // 加载世界资源
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void LoadWorldResources();
    virtual void LoadWorldResources_Implementation();

    // 初始化世界状态
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeWorldState();
    virtual void InitializeWorldState_Implementation();

    // 初始化玩家
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializePlayers();
    virtual void InitializePlayers_Implementation();

    // 后初始化（在主要初始化之后）
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PostInitializeWorld();
    virtual void PostInitializeWorld_Implementation();

    // ========== 关闭步骤（可重写） ==========

    // 预关闭
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void PreShutdownWorld();
    virtual void PreShutdownWorld_Implementation();

    // 保存世界状态
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void SaveWorldState();
    virtual void SaveWorldState_Implementation();

    // 清理资源
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void CleanupWorldResources();
    virtual void CleanupWorldResources_Implementation();

    // 后关闭
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
    // ========== 内部实现 ==========

    // 内部初始化完成处理 - 移除UFUNCTION，因为不需要蓝图调用
    void HandleWorldInitialized(bool bSuccess);

    // 异步初始化计时器回调 - 移除UFUNCTION
    void ExecuteAsyncInitialization();

    // 生成默认玩家
    APawn* SpawnDefaultPlayer(AController* NewPlayer);

    // 获取随机生成点
    FTransform GetRandomSpawnTransform() const;

private:
    // 世界初始化状态
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State", meta = (AllowPrivateAccess = "true"))
    EWorldInitState WorldInitState;

    // 世界配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Config", meta = (AllowPrivateAccess = "true"))
    FWorldConfig WorldConfiguration;

    // 玩家生成配置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Player", meta = (AllowPrivateAccess = "true"))
    FPlayerSpawnConfig PlayerSpawnConfiguration;

    // 回调存储
    TArray<FOnWorldInitCallback> InitCallbacks;

    // 异步初始化计时器ID
    FString AsyncInitTimerId;
};