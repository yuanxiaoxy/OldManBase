// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SaveGame.h"
#include "Engine/Engine.h"
#include "SaveGameDataTypes.h"
#include "SaveGameTool.generated.h"

// 存档槽类型
UENUM(BlueprintType)
enum class ESaveSlotType : uint8
{
    AutoSave     UMETA(DisplayName = "Auto Save"),
    ManualSave   UMETA(DisplayName = "Manual Save"),
    QuickSave    UMETA(DisplayName = "Quick Save"),
    Checkpoint   UMETA(DisplayName = "Checkpoint"),
    System       UMETA(DisplayName = "System Data")
};

// 存档数据状态
UENUM(BlueprintType)
enum class ESaveDataState : uint8
{
    Valid        UMETA(DisplayName = "Valid"),
    Corrupted    UMETA(DisplayName = "Corrupted"),
    Outdated     UMETA(DisplayName = "Outdated"),
    Loading      UMETA(DisplayName = "Loading"),
    Saving       UMETA(DisplayName = "Saving")
};

// 存档元数据
USTRUCT(BlueprintType)
struct FSaveGameMetadata
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FString SaveSlotName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FDateTime SaveDateTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    float PlayTimeSeconds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FString LevelName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    ESaveSlotType SlotType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    int32 SaveVersion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FString GameVersion;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FString PlayerName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    int32 PlayerLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FString ThumbnailPath;

    FSaveGameMetadata()
        : PlayTimeSeconds(0.0f)
        , SlotType(ESaveSlotType::ManualSave)
        , SaveVersion(1)
        , PlayerLevel(1)
    {
        SaveDateTime = FDateTime::Now();
    }
};

// 基础存档类
UCLASS(Blueprintable, BlueprintType)
class USaveGameBase : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    FSaveGameMetadata Metadata;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
    TMap<FString, FString> CustomStringData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
    TMap<FString, float> CustomFloatData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
    TMap<FString, int32> CustomIntData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
    TMap<FString, bool> CustomBoolData;
};

// 玩家存档类
UCLASS(Blueprintable)
class UPlayerSaveGame : public USaveGameBase
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player")
    FPlayerSaveData PlayerData;

    // 便捷方法
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetPlayerData(const FPlayerSaveData& NewData) { PlayerData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "Player")
    FPlayerSaveData GetPlayerData() const { return PlayerData; }
};

// 世界存档类
UCLASS(Blueprintable)
class UWorldSaveGame : public USaveGameBase
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "World")
    FWorldSaveData WorldData;

    UFUNCTION(BlueprintCallable, Category = "World")
    void SetWorldData(const FWorldSaveData& NewData) { WorldData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "World")
    FWorldSaveData GetWorldData() const { return WorldData; }
};

// 设置存档类
UCLASS(Blueprintable)
class USettingsSaveGame : public USaveGameBase
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Settings")
    FSettingsData SettingsData;

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetSettingsData(const FSettingsData& NewData) { SettingsData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    FSettingsData GetSettingsData() const { return SettingsData; }
};

// 进度存档类
UCLASS(Blueprintable)
class UProgressSaveGame : public USaveGameBase
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Progress")
    FProgressData ProgressData;

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void SetProgressData(const FProgressData& NewData) { ProgressData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "Progress")
    FProgressData GetProgressData() const { return ProgressData; }
};

// 存档完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameComplete, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoadGameComplete, const FString&, SlotName, USaveGameBase*, SaveGame, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeleteSaveComplete, const FString&, SlotName, bool, bSuccess);

// 蓝图回调委托
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSaveGameCallback, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnLoadGameCallback, const FString&, SlotName, USaveGameBase*, SaveGame, bool, bSuccess);

// 静态委托 - 用于C++回调
DECLARE_DELEGATE_TwoParams(FOnSaveGameStaticDelegate, const FString&, bool);
DECLARE_DELEGATE_ThreeParams(FOnLoadGameStaticDelegate, const FString&, USaveGameBase*, bool);

// 加载结果结构体
USTRUCT(BlueprintType)
struct FLoadResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "SaveGame")
    bool bSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "SaveGame")
    FString ErrorMessage;

    FLoadResult() : bSuccess(false) {}
    FLoadResult(bool Success, const FString& Error = TEXT("")) : bSuccess(Success), ErrorMessage(Error) {}
};

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API USaveGameTool : public USingletonBase
{
    GENERATED_BODY()

    // 单例声明
    DECLARE_SINGLETON(USaveGameTool)

public:
    // 构造函数和析构函数
    USaveGameTool();
    virtual ~USaveGameTool() override;

    // 初始化存档管理器
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void InitializeSaveTool();

    // 单例初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // 获取单例的可访问方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveGame", meta = (DisplayName = "Get Save Game Tool"))
    static USaveGameTool* GetSaveGameTool() { return GetInstance(); }

    // ========== 基础存档操作 ==========

    // 同步保存游戏
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    bool SaveGameSync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex = 0);

    // 同步加载游戏
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    USaveGameBase* LoadGameSync(const FString& SlotName, int32 UserIndex = 0);

    // 同步删除存档
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    bool DeleteGameSync(const FString& SlotName, int32 UserIndex = 0);

    // ========== 异步存档操作 ==========

    // 异步保存游戏
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void SaveGameAsync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex = 0);

    // 异步加载游戏
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void LoadGameAsync(const FString& SlotName, int32 UserIndex = 0);

    // 异步删除存档
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void DeleteGameAsync(const FString& SlotName, int32 UserIndex = 0);

    // ========== 带回调的异步操作 ==========

    // 异步保存游戏（蓝图回调）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameCallback& Callback, int32 UserIndex = 0);

    // 异步加载游戏（蓝图回调）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void LoadGameAsyncWithCallback(const FString& SlotName, const FOnLoadGameCallback& Callback, int32 UserIndex = 0);

    // ========== C++静态委托回调方法 ==========

    // 异步保存游戏（C++静态委托）
    template<typename T>
    void SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, T* Object, void(T::* Function)(const FString&, bool), int32 UserIndex = 0)
    {
        FOnSaveGameStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalSaveGameAsyncWithStaticCallback(SlotName, SaveGameObject, StaticDelegate, UserIndex);
    }

    // 异步保存游戏（无参数C++静态委托）
    template<typename T>
    void SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, T* Object, void(T::* Function)(), int32 UserIndex = 0)
    {
        FOnSaveGameStaticDelegate StaticDelegate = FOnSaveGameStaticDelegate::CreateLambda([Object, Function](const FString& SlotName, bool bSuccess) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalSaveGameAsyncWithStaticCallback(SlotName, SaveGameObject, StaticDelegate, UserIndex);
    }

    // 异步加载游戏（C++静态委托）
    template<typename T>
    void LoadGameAsyncWithCallback(const FString& SlotName, T* Object, void(T::* Function)(const FString&, USaveGameBase*, bool), int32 UserIndex = 0)
    {
        FOnLoadGameStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadGameAsyncWithStaticCallback(SlotName, StaticDelegate, UserIndex);
    }

    // 异步加载游戏（无参数C++静态委托）
    template<typename T>
    void LoadGameAsyncWithCallback(const FString& SlotName, T* Object, void(T::* Function)(), int32 UserIndex = 0)
    {
        FOnLoadGameStaticDelegate StaticDelegate = FOnLoadGameStaticDelegate::CreateLambda([Object, Function](const FString& SlotName, USaveGameBase* SaveGame, bool bSuccess) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadGameAsyncWithStaticCallback(SlotName, StaticDelegate, UserIndex);
    }

    // ========== 快速存档功能 ==========

    // 快速保存
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Quick")
    void QuickSave();

    // 快速加载
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Quick")
    bool QuickLoad();

    // 自动保存
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Auto")
    void AutoSave();

    // ========== 数据创建和获取 ==========

    // 创建空的存档对象
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UPlayerSaveGame* CreatePlayerSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UWorldSaveGame* CreateWorldSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* CreateSettingsSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UProgressSaveGame* CreateProgressSaveGame();

    // 从结构体创建存档对象
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UPlayerSaveGame* CreatePlayerSaveGameFromData(const FPlayerSaveData& PlayerData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UWorldSaveGame* CreateWorldSaveGameFromData(const FWorldSaveData& WorldData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* CreateSettingsSaveGameFromData(const FSettingsData& SettingsData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UProgressSaveGame* CreateProgressSaveGameFromData(const FProgressData& ProgressData);

    // 从存档对象获取结构体数据
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FPlayerSaveData GetPlayerDataFromSaveGame(UPlayerSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FWorldSaveData GetWorldDataFromSaveGame(UWorldSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FSettingsData GetSettingsDataFromSaveGame(USettingsSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FProgressData GetProgressDataFromSaveGame(UProgressSaveGame* SaveGame) const;

    // ========== 安全的加载方法（返回bool表示成功） ==========

    // 安全加载玩家数据到结构体
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadPlayerDataStruct(FPlayerSaveData& OutData, const FString& SlotName = TEXT("PlayerData"));

    // 安全加载世界数据到结构体
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadWorldDataStruct(FWorldSaveData& OutData, const FString& SlotName = TEXT("WorldData"));

    // 安全加载设置数据到结构体
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadSettingsDataStruct(FSettingsData& OutData, const FString& SlotName = TEXT("GameSettings"));

    // 安全加载进度数据到结构体
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadProgressDataStruct(FProgressData& OutData, const FString& SlotName = TEXT("ProgressData"));

    // ========== 直接结构体保存 ==========

    // 直接使用结构体保存数据
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SavePlayerDataStruct(const FPlayerSaveData& PlayerData, const FString& SlotName = TEXT("PlayerData"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveWorldDataStruct(const FWorldSaveData& WorldData, const FString& SlotName = TEXT("WorldData"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveSettingsDataStruct(const FSettingsData& SettingsData, const FString& SlotName = TEXT("GameSettings"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveProgressDataStruct(const FProgressData& ProgressData, const FString& SlotName = TEXT("ProgressData"));

    // ========== 便捷保存方法 ==========

    // 保存玩家数据（从Pawn创建数据）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SavePlayerData(APawn* PlayerPawn, const FString& SlotName = TEXT("PlayerData"));

    // 加载玩家数据到Pawn
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadPlayerData(APawn* PlayerPawn, const FString& SlotName = TEXT("PlayerData"));

    // 从Pawn创建玩家数据
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool CreatePlayerDataFromPawn(APawn* PlayerPawn, FPlayerSaveData& OutData);

    // ========== 当前数据管理 ==========

    // 获取当前设置（如果没有则加载或创建默认设置）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* GetCurrentSettings();

    // 设置当前设置
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    void SetCurrentSettings(USettingsSaveGame* NewSettings);

    // 获取当前设置数据
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FSettingsData GetCurrentSettingsData() const;

    // 设置当前设置数据
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    void SetCurrentSettingsData(const FSettingsData& NewData);

    // 保存当前设置
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveCurrentSettings(const FString& SlotName = TEXT("GameSettings"));

    // 加载设置
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadSettings(const FString& SlotName = TEXT("GameSettings"));

    // ========== 存档管理 ==========

    // 检查存档是否存在
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    bool DoesSaveGameExist(const FString& SlotName, int32 UserIndex = 0);

    // 获取所有存档槽
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    TArray<FString> GetAllSaveSlots() const;

    // 获取存档元数据（不加载完整存档）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    FSaveGameMetadata GetSaveMetadata(const FString& SlotName, int32 UserIndex = 0);

    // 获取存档大小（字节）
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    int64 GetSaveGameSize(const FString& SlotName, int32 UserIndex = 0) const;

    // 获取总存档大小
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    int64 GetTotalSaveSize() const;

    // 清理旧存档
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    void CleanupOldSaves(int32 MaxSaveCount = 10);

    // ========== 调试工具 ==========

    // 打印所有存档
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    void PrintAllSaves();

    // 验证存档完整性
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    bool ValidateSaveGame(const FString& SlotName, int32 UserIndex = 0);

    // 打印存档统计信息
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    void PrintSaveStatistics();

    // ========== 事件委托 ==========

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnSaveGameComplete OnSaveGameComplete;

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnLoadGameComplete OnLoadGameComplete;

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnDeleteSaveComplete OnDeleteSaveComplete;

private:
    // 当前设置
    UPROPERTY()
    USettingsSaveGame* CurrentSettings;

    // 当前设置数据缓存
    FSettingsData CurrentSettingsData;

    // 回调映射
    TMap<FString, FOnSaveGameCallback> SaveCallbacks;
    TMap<FString, FOnLoadGameCallback> LoadCallbacks;

    // 静态委托映射
    TMap<FString, FOnSaveGameStaticDelegate> SaveStaticCallbacks;
    TMap<FString, FOnLoadGameStaticDelegate> LoadStaticCallbacks;

    // 内部实现方法
    FString GenerateBackupName(const FString& SlotName) const;
    bool InternalSaveGame(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex);
    USaveGameBase* InternalLoadGame(const FString& SlotName, int32 UserIndex);
    void UpdateSaveMetadata(USaveGameBase* SaveGame, const FString& SlotName, ESaveSlotType SlotType);

    // 静态委托内部方法
    void InternalSaveGameAsyncWithStaticCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameStaticDelegate& Callback, int32 UserIndex);
    void InternalLoadGameAsyncWithStaticCallback(const FString& SlotName, const FOnLoadGameStaticDelegate& Callback, int32 UserIndex);

    // 异步操作完成处理
    UFUNCTION()
    void HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);

    UFUNCTION()
    void HandleAsyncLoadComplete(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame);

    // 获取世界
    UWorld* GetWorld() const override;
};