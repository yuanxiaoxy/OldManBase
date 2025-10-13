// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveManager/SaveGameTool.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// 静态实例定义
template<>
USaveGameTool* TSingleton<USaveGameTool>::SingletonInstance = nullptr;

USaveGameTool::USaveGameTool()
    : CurrentSettings(nullptr)
{
}

USaveGameTool::~USaveGameTool()
{
    // 清理资源
    if (CurrentSettings)
    {
        CurrentSettings = nullptr;
    }

    SaveCallbacks.Empty();
    LoadCallbacks.Empty();
    SaveStaticCallbacks.Empty();
    LoadStaticCallbacks.Empty();
}

void USaveGameTool::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("SaveGameTool InitializeSingleton called"));
    InitializeSaveTool();
}

void USaveGameTool::InitializeSaveTool()
{
    UE_LOG(LogTemp, Log, TEXT("Save Game Tool Initialized"));

    // 加载默认设置
    LoadSettings();

    UE_LOG(LogTemp, Log, TEXT("SaveGameTool ready"));
}

// ========== 基础存档操作 ==========

bool USaveGameTool::SaveGameSync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex)
{
    if (!SaveGameObject || SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid save game parameters"));
        return false;
    }

    // 更新元数据
    UpdateSaveMetadata(SaveGameObject, SlotName, ESaveSlotType::ManualSave);

    return InternalSaveGame(SlotName, SaveGameObject, UserIndex);
}

USaveGameBase* USaveGameTool::LoadGameSync(const FString& SlotName, int32 UserIndex)
{
    if (SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot name"));
        return nullptr;
    }

    return InternalLoadGame(SlotName, UserIndex);
}

bool USaveGameTool::DeleteGameSync(const FString& SlotName, int32 UserIndex)
{
    if (SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot name"));
        return false;
    }

    return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

// ========== 异步存档操作 ==========

void USaveGameTool::SaveGameAsync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex)
{
    if (!SaveGameObject || SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid save game parameters"));
        return;
    }

    // 更新元数据
    UpdateSaveMetadata(SaveGameObject, SlotName, ESaveSlotType::ManualSave);

    // 使用UE的异步保存系统
    FAsyncSaveGameToSlotDelegate SavedDelegate;
    SavedDelegate.BindUObject(this, &USaveGameTool::HandleAsyncSaveComplete);
    UGameplayStatics::AsyncSaveGameToSlot(SaveGameObject, SlotName, UserIndex, SavedDelegate);
}

void USaveGameTool::LoadGameAsync(const FString& SlotName, int32 UserIndex)
{
    if (SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid slot name"));
        return;
    }

    // 使用UE的异步加载系统
    FAsyncLoadGameFromSlotDelegate LoadedDelegate;
    LoadedDelegate.BindUObject(this, &USaveGameTool::HandleAsyncLoadComplete);
    UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, LoadedDelegate);
}

void USaveGameTool::DeleteGameAsync(const FString& SlotName, int32 UserIndex)
{
    // 删除操作通常是同步的，但我们可以在后台线程中执行
    Async(EAsyncExecution::Thread, [this, SlotName, UserIndex]()
        {
            bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

            // 回到游戏线程广播事件
            Async(EAsyncExecution::TaskGraphMainThread, [this, SlotName, bSuccess]()
                {
                    OnDeleteSaveComplete.Broadcast(SlotName, bSuccess);
                });
        });
}

// ========== 带回调的异步操作 ==========

void USaveGameTool::SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameCallback& Callback, int32 UserIndex)
{
    if (Callback.IsBound())
    {
        SaveCallbacks.Add(SlotName, Callback);
    }

    SaveGameAsync(SlotName, SaveGameObject, UserIndex);
}

void USaveGameTool::LoadGameAsyncWithCallback(const FString& SlotName, const FOnLoadGameCallback& Callback, int32 UserIndex)
{
    if (Callback.IsBound())
    {
        LoadCallbacks.Add(SlotName, Callback);
    }

    LoadGameAsync(SlotName, UserIndex);
}

// ========== 静态委托内部实现 ==========

void USaveGameTool::InternalSaveGameAsyncWithStaticCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameStaticDelegate& Callback, int32 UserIndex)
{
    if (Callback.IsBound())
    {
        SaveStaticCallbacks.Add(SlotName, Callback);
    }

    SaveGameAsync(SlotName, SaveGameObject, UserIndex);
}

void USaveGameTool::InternalLoadGameAsyncWithStaticCallback(const FString& SlotName, const FOnLoadGameStaticDelegate& Callback, int32 UserIndex)
{
    if (Callback.IsBound())
    {
        LoadStaticCallbacks.Add(SlotName, Callback);
    }

    LoadGameAsync(SlotName, UserIndex);
}

// ========== 快速存档功能 ==========

void USaveGameTool::QuickSave()
{
    FString QuickSaveSlot = TEXT("QuickSave_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    // 创建快速存档 - 使用新的结构体保存方法
    FPlayerSaveData PlayerData;
    // 这里可以设置一些默认的快速存档数据
    if (SavePlayerDataStruct(PlayerData, QuickSaveSlot))
    {
        UE_LOG(LogTemp, Log, TEXT("QuickSave created: %s"), *QuickSaveSlot);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create QuickSave: %s"), *QuickSaveSlot);
    }
}

bool USaveGameTool::QuickLoad()
{
    TArray<FString> AllSaves = GetAllSaveSlots();
    FString LatestQuickSave;
    FDateTime LatestTime = FDateTime::MinValue();

    // 查找最新的快速存档
    for (const FString& Slot : AllSaves)
    {
        if (Slot.StartsWith(TEXT("QuickSave_")))
        {
            FSaveGameMetadata Metadata = GetSaveMetadata(Slot);
            if (Metadata.SaveDateTime > LatestTime)
            {
                LatestTime = Metadata.SaveDateTime;
                LatestQuickSave = Slot;
            }
        }
    }

    if (!LatestQuickSave.IsEmpty())
    {
        FPlayerSaveData PlayerData;
        if (LoadPlayerDataStruct(PlayerData, LatestQuickSave))
        {
            UE_LOG(LogTemp, Log, TEXT("QuickLoad from: %s"), *LatestQuickSave);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No QuickSave found to load"));
    return false;
}

void USaveGameTool::AutoSave()
{
    FString AutoSaveSlot = TEXT("AutoSave_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    // 创建自动存档 - 使用结构体保存方法
    FPlayerSaveData PlayerData;
    // 这里可以设置一些自动存档数据
    if (SavePlayerDataStruct(PlayerData, AutoSaveSlot))
    {
        UE_LOG(LogTemp, Log, TEXT("AutoSave created: %s"), *AutoSaveSlot);

        // 清理旧的自动存档（保留最近5个）
        CleanupOldSaves(5);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create AutoSave: %s"), *AutoSaveSlot);
    }
}

// ========== 数据创建和获取 ==========

UPlayerSaveGame* USaveGameTool::CreatePlayerSaveGame()
{
    return NewObject<UPlayerSaveGame>();
}

UWorldSaveGame* USaveGameTool::CreateWorldSaveGame()
{
    return NewObject<UWorldSaveGame>();
}

USettingsSaveGame* USaveGameTool::CreateSettingsSaveGame()
{
    USettingsSaveGame* SettingsSave = NewObject<USettingsSaveGame>();
    SettingsSave->Metadata.SlotType = ESaveSlotType::System;
    return SettingsSave;
}

UProgressSaveGame* USaveGameTool::CreateProgressSaveGame()
{
    UProgressSaveGame* ProgressSave = NewObject<UProgressSaveGame>();
    ProgressSave->Metadata.SlotType = ESaveSlotType::System;
    return ProgressSave;
}

// ========== 结构体数据操作 ==========

UPlayerSaveGame* USaveGameTool::CreatePlayerSaveGameFromData(const FPlayerSaveData& PlayerData)
{
    UPlayerSaveGame* SaveGame = CreatePlayerSaveGame();
    SaveGame->SetPlayerData(PlayerData);
    return SaveGame;
}

UWorldSaveGame* USaveGameTool::CreateWorldSaveGameFromData(const FWorldSaveData& WorldData)
{
    UWorldSaveGame* SaveGame = CreateWorldSaveGame();
    SaveGame->SetWorldData(WorldData);
    return SaveGame;
}

USettingsSaveGame* USaveGameTool::CreateSettingsSaveGameFromData(const FSettingsData& SettingsData)
{
    USettingsSaveGame* SaveGame = CreateSettingsSaveGame();
    SaveGame->SetSettingsData(SettingsData);
    return SaveGame;
}

UProgressSaveGame* USaveGameTool::CreateProgressSaveGameFromData(const FProgressData& ProgressData)
{
    UProgressSaveGame* SaveGame = CreateProgressSaveGame();
    SaveGame->SetProgressData(ProgressData);
    return SaveGame;
}

FPlayerSaveData USaveGameTool::GetPlayerDataFromSaveGame(UPlayerSaveGame* SaveGame) const
{
    return SaveGame ? SaveGame->GetPlayerData() : FPlayerSaveData();
}

FWorldSaveData USaveGameTool::GetWorldDataFromSaveGame(UWorldSaveGame* SaveGame) const
{
    return SaveGame ? SaveGame->GetWorldData() : FWorldSaveData();
}

FSettingsData USaveGameTool::GetSettingsDataFromSaveGame(USettingsSaveGame* SaveGame) const
{
    return SaveGame ? SaveGame->GetSettingsData() : FSettingsData();
}

FProgressData USaveGameTool::GetProgressDataFromSaveGame(UProgressSaveGame* SaveGame) const
{
    return SaveGame ? SaveGame->GetProgressData() : FProgressData();
}

// ========== 安全的加载方法（返回bool表示成功） ==========

bool USaveGameTool::LoadPlayerDataStruct(FPlayerSaveData& OutData, const FString& SlotName)
{
    UPlayerSaveGame* SaveGame = Cast<UPlayerSaveGame>(LoadGameSync(SlotName));
    if (SaveGame)
    {
        OutData = SaveGame->GetPlayerData();
        return true;
    }
    return false;
}

bool USaveGameTool::LoadWorldDataStruct(FWorldSaveData& OutData, const FString& SlotName)
{
    UWorldSaveGame* SaveGame = Cast<UWorldSaveGame>(LoadGameSync(SlotName));
    if (SaveGame)
    {
        OutData = SaveGame->GetWorldData();
        return true;
    }
    return false;
}

bool USaveGameTool::LoadSettingsDataStruct(FSettingsData& OutData, const FString& SlotName)
{
    USettingsSaveGame* SaveGame = Cast<USettingsSaveGame>(LoadGameSync(SlotName));
    if (SaveGame)
    {
        OutData = SaveGame->GetSettingsData();
        return true;
    }
    return false;
}

bool USaveGameTool::LoadProgressDataStruct(FProgressData& OutData, const FString& SlotName)
{
    UProgressSaveGame* SaveGame = Cast<UProgressSaveGame>(LoadGameSync(SlotName));
    if (SaveGame)
    {
        OutData = SaveGame->GetProgressData();
        return true;
    }
    return false;
}

// ========== 直接结构体保存 ==========

bool USaveGameTool::SavePlayerDataStruct(const FPlayerSaveData& PlayerData, const FString& SlotName)
{
    UPlayerSaveGame* SaveGame = CreatePlayerSaveGameFromData(PlayerData);
    if (SaveGame)
    {
        return SaveGameSync(SlotName, SaveGame);
    }
    return false;
}

bool USaveGameTool::SaveWorldDataStruct(const FWorldSaveData& WorldData, const FString& SlotName)
{
    UWorldSaveGame* SaveGame = CreateWorldSaveGameFromData(WorldData);
    if (SaveGame)
    {
        return SaveGameSync(SlotName, SaveGame);
    }
    return false;
}

bool USaveGameTool::SaveSettingsDataStruct(const FSettingsData& SettingsData, const FString& SlotName)
{
    USettingsSaveGame* SaveGame = CreateSettingsSaveGameFromData(SettingsData);
    if (SaveGame)
    {
        return SaveGameSync(SlotName, SaveGame);
    }
    return false;
}

bool USaveGameTool::SaveProgressDataStruct(const FProgressData& ProgressData, const FString& SlotName)
{
    UProgressSaveGame* SaveGame = CreateProgressSaveGameFromData(ProgressData);
    if (SaveGame)
    {
        return SaveGameSync(SlotName, SaveGame);
    }
    return false;
}

// ========== 便捷保存方法 ==========

bool USaveGameTool::SavePlayerData(APawn* PlayerPawn, const FString& SlotName)
{
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid player pawn for save"));
        return false;
    }

    FPlayerSaveData PlayerData;
    if (CreatePlayerDataFromPawn(PlayerPawn, PlayerData))
    {
        return SavePlayerDataStruct(PlayerData, SlotName);
    }
    return false;
}

bool USaveGameTool::LoadPlayerData(APawn* PlayerPawn, const FString& SlotName)
{
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid player pawn for load"));
        return false;
    }

    FPlayerSaveData PlayerData;
    if (LoadPlayerDataStruct(PlayerData, SlotName))
    {
        // 应用玩家数据到Pawn
        PlayerPawn->SetActorLocation(PlayerData.Location);
        PlayerPawn->SetActorRotation(PlayerData.Rotation);
        UE_LOG(LogTemp, Log, TEXT("Player data loaded from: %s"), *SlotName);
        return true;
    }
    return false;
}

bool USaveGameTool::CreatePlayerDataFromPawn(APawn* PlayerPawn, FPlayerSaveData& OutData)
{
    if (!PlayerPawn)
    {
        return false;
    }

    // 保存玩家位置和旋转
    OutData.Location = PlayerPawn->GetActorLocation();
    OutData.Rotation = PlayerPawn->GetActorRotation();

    // 可以在这里添加更多从Pawn获取数据的逻辑
    return true;
}

// ========== 当前数据管理 ==========

USettingsSaveGame* USaveGameTool::GetCurrentSettings()
{
    if (!CurrentSettings)
    {
        // 如果没有当前设置，尝试加载或创建默认设置
        if (!LoadSettings())
        {
            CurrentSettings = CreateSettingsSaveGame();
        }
    }
    return CurrentSettings;
}

void USaveGameTool::SetCurrentSettings(USettingsSaveGame* NewSettings)
{
    if (NewSettings)
    {
        CurrentSettings = NewSettings;
        CurrentSettingsData = NewSettings->GetSettingsData();
        UE_LOG(LogTemp, Log, TEXT("Current settings updated"));
    }
}

FSettingsData USaveGameTool::GetCurrentSettingsData() const
{
    return CurrentSettingsData;
}

void USaveGameTool::SetCurrentSettingsData(const FSettingsData& NewData)
{
    CurrentSettingsData = NewData;
    if (CurrentSettings)
    {
        CurrentSettings->SetSettingsData(NewData);
    }
}

bool USaveGameTool::SaveCurrentSettings(const FString& SlotName)
{
    if (CurrentSettings)
    {
        return SaveGameSync(SlotName, CurrentSettings);
    }
    return false;
}

bool USaveGameTool::LoadSettings(const FString& SlotName)
{
    FSettingsData SettingsData;
    if (LoadSettingsDataStruct(SettingsData, SlotName))
    {
        CurrentSettingsData = SettingsData;
        if (!CurrentSettings)
        {
            CurrentSettings = CreateSettingsSaveGame();
        }
        CurrentSettings->SetSettingsData(SettingsData);
        UE_LOG(LogTemp, Log, TEXT("Settings loaded from: %s"), *SlotName);
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("No settings found at: %s"), *SlotName);
    return false;
}

// ========== 存档管理 ==========

bool USaveGameTool::DoesSaveGameExist(const FString& SlotName, int32 UserIndex)
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

TArray<FString> USaveGameTool::GetAllSaveSlots() const
{
    TArray<FString> SaveSlots;

    // 获取存档目录
    FString SaveGameDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
    TArray<FString> SaveFiles;

    // 查找所有.sav文件
    IFileManager::Get().FindFiles(SaveFiles, *SaveGameDir, TEXT(".sav"));

    for (const FString& SaveFile : SaveFiles)
    {
        // 移除.sav扩展名
        FString SlotName = FPaths::GetBaseFilename(SaveFile);
        SaveSlots.Add(SlotName);
    }

    return SaveSlots;
}

FSaveGameMetadata USaveGameTool::GetSaveMetadata(const FString& SlotName, int32 UserIndex)
{
    FSaveGameMetadata Metadata;

    USaveGameBase* SaveData = InternalLoadGame(SlotName, UserIndex);
    if (SaveData)
    {
        Metadata = SaveData->Metadata;
    }

    return Metadata;
}

int64 USaveGameTool::GetSaveGameSize(const FString& SlotName, int32 UserIndex) const
{
    FString SaveFilePath = FPaths::ProjectSavedDir() / TEXT("SaveGames") / (SlotName + TEXT(".sav"));

    if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*SaveFilePath))
    {
        return IFileManager::Get().FileSize(*SaveFilePath);
    }

    return 0;
}

int64 USaveGameTool::GetTotalSaveSize() const
{
    int64 TotalSize = 0;
    TArray<FString> SaveSlots = GetAllSaveSlots();

    for (const FString& Slot : SaveSlots)
    {
        TotalSize += GetSaveGameSize(Slot);
    }

    return TotalSize;
}

void USaveGameTool::CleanupOldSaves(int32 MaxSaveCount)
{
    TArray<FString> AllSaves = GetAllSaveSlots();
    TArray<FSaveGameMetadata> SaveMetadatas;

    // 收集所有存档的元数据
    for (const FString& Slot : AllSaves)
    {
        FSaveGameMetadata Metadata = GetSaveMetadata(Slot);
        if (Metadata.SaveSlotName == Slot) // 验证元数据有效
        {
            SaveMetadatas.Add(Metadata);
        }
    }

    // 按时间排序（最新的在前）
    SaveMetadatas.Sort([](const FSaveGameMetadata& A, const FSaveGameMetadata& B)
        {
            return A.SaveDateTime > B.SaveDateTime;
        });

    // 删除超过数量限制的旧存档
    for (int32 i = MaxSaveCount; i < SaveMetadatas.Num(); i++)
    {
        DeleteGameSync(SaveMetadatas[i].SaveSlotName);
        UE_LOG(LogTemp, Log, TEXT("Cleaned up old save: %s"), *SaveMetadatas[i].SaveSlotName);
    }
}

// ========== 调试工具 ==========

void USaveGameTool::PrintAllSaves()
{
    TArray<FString> SaveSlots = GetAllSaveSlots();

    UE_LOG(LogTemp, Log, TEXT("=== All Save Games (%d) ==="), SaveSlots.Num());

    for (const FString& Slot : SaveSlots)
    {
        FSaveGameMetadata Metadata = GetSaveMetadata(Slot);
        int64 Size = GetSaveGameSize(Slot);

        UE_LOG(LogTemp, Log, TEXT("  %s - %s - %s - %.2f KB"),
            *Slot,
            *UEnum::GetValueAsString(Metadata.SlotType),
            *Metadata.SaveDateTime.ToString(),
            Size / 1024.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Save Games ==="));
}

bool USaveGameTool::ValidateSaveGame(const FString& SlotName, int32 UserIndex)
{
    USaveGameBase* SaveData = InternalLoadGame(SlotName, UserIndex);
    if (!SaveData)
    {
        return false;
    }

    // 基本验证
    bool bValid = !SaveData->Metadata.SaveSlotName.IsEmpty() &&
        SaveData->Metadata.SaveDateTime.GetTicks() > 0;

    UE_LOG(LogTemp, Log, TEXT("Save validation for %s: %s"), *SlotName, bValid ? TEXT("VALID") : TEXT("INVALID"));

    return bValid;
}

void USaveGameTool::PrintSaveStatistics()
{
    TArray<FString> SaveSlots = GetAllSaveSlots();
    int64 TotalSize = GetTotalSaveSize();

    UE_LOG(LogTemp, Log, TEXT("=== Save Game Statistics ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Saves: %d"), SaveSlots.Num());
    UE_LOG(LogTemp, Log, TEXT("Total Size: %.2f MB"), TotalSize / (1024.0f * 1024.0f));
    UE_LOG(LogTemp, Log, TEXT("=== End Statistics ==="));
}

// ========== 内部实现 ==========

FString USaveGameTool::GenerateBackupName(const FString& SlotName) const
{
    return SlotName + TEXT("_Backup_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
}

bool USaveGameTool::InternalSaveGame(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex)
{
    if (!SaveGameObject)
    {
        return false;
    }

    // 使用UE的保存系统
    return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, UserIndex);
}

USaveGameBase* USaveGameTool::InternalLoadGame(const FString& SlotName, int32 UserIndex)
{
    return Cast<USaveGameBase>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
}

void USaveGameTool::UpdateSaveMetadata(USaveGameBase* SaveGame, const FString& SlotName, ESaveSlotType SlotType)
{
    if (!SaveGame)
    {
        return;
    }

    SaveGame->Metadata.SaveSlotName = SlotName;
    SaveGame->Metadata.SaveDateTime = FDateTime::Now();
    SaveGame->Metadata.SlotType = SlotType;

    UWorld* World = GetWorld();
    if (World)
    {
        SaveGame->Metadata.LevelName = UGameplayStatics::GetCurrentLevelName(World);
        SaveGame->Metadata.PlayTimeSeconds = World->GetTimeSeconds();
    }

    // 设置游戏版本
    SaveGame->Metadata.GameVersion = TEXT("1.0.0");
    SaveGame->Metadata.SaveVersion = 1;
}

void USaveGameTool::HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Async save completed: %s - %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failed"));

    // 广播委托
    OnSaveGameComplete.Broadcast(SlotName, bSuccess);

    // 执行蓝图回调
    FOnSaveGameCallback* Callback = SaveCallbacks.Find(SlotName);
    if (Callback && Callback->IsBound())
    {
        Callback->Execute(SlotName, bSuccess);
        SaveCallbacks.Remove(SlotName);
    }

    // 执行静态委托回调
    FOnSaveGameStaticDelegate* StaticCallback = SaveStaticCallbacks.Find(SlotName);
    if (StaticCallback && StaticCallback->IsBound())
    {
        StaticCallback->Execute(SlotName, bSuccess);
        SaveStaticCallbacks.Remove(SlotName);
    }
}

void USaveGameTool::HandleAsyncLoadComplete(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame)
{
    USaveGameBase* SaveGameBase = Cast<USaveGameBase>(SaveGame);
    bool bSuccess = (SaveGameBase != nullptr);

    UE_LOG(LogTemp, Log, TEXT("Async load completed: %s - %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failed"));

    // 广播委托
    OnLoadGameComplete.Broadcast(SlotName, SaveGameBase, bSuccess);

    // 执行蓝图回调
    FOnLoadGameCallback* Callback = LoadCallbacks.Find(SlotName);
    if (Callback && Callback->IsBound())
    {
        Callback->Execute(SlotName, SaveGameBase, bSuccess);
        LoadCallbacks.Remove(SlotName);
    }

    // 执行静态委托回调
    FOnLoadGameStaticDelegate* StaticCallback = LoadStaticCallbacks.Find(SlotName);
    if (StaticCallback && StaticCallback->IsBound())
    {
        StaticCallback->Execute(SlotName, SaveGameBase, bSuccess);
        LoadStaticCallbacks.Remove(SlotName);
    }
}

UWorld* USaveGameTool::GetWorld() const
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