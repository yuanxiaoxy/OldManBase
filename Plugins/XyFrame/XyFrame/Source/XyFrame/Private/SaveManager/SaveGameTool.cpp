// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveManager/SaveGameTool.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

// ��̬ʵ������
template<>
USaveGameTool* TSingleton<USaveGameTool>::SingletonInstance = nullptr;

USaveGameTool::USaveGameTool()
    : CurrentSettings(nullptr)
{
}

USaveGameTool::~USaveGameTool()
{
    // ������Դ
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

    // ����Ĭ������
    LoadSettings();

    UE_LOG(LogTemp, Log, TEXT("SaveGameTool ready"));
}

// ========== �����浵���� ==========

bool USaveGameTool::SaveGameSync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex)
{
    if (!SaveGameObject || SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid save game parameters"));
        return false;
    }

    // ����Ԫ����
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

// ========== �첽�浵���� ==========

void USaveGameTool::SaveGameAsync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex)
{
    if (!SaveGameObject || SlotName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid save game parameters"));
        return;
    }

    // ����Ԫ����
    UpdateSaveMetadata(SaveGameObject, SlotName, ESaveSlotType::ManualSave);

    // ʹ��UE���첽����ϵͳ
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

    // ʹ��UE���첽����ϵͳ
    FAsyncLoadGameFromSlotDelegate LoadedDelegate;
    LoadedDelegate.BindUObject(this, &USaveGameTool::HandleAsyncLoadComplete);
    UGameplayStatics::AsyncLoadGameFromSlot(SlotName, UserIndex, LoadedDelegate);
}

void USaveGameTool::DeleteGameAsync(const FString& SlotName, int32 UserIndex)
{
    // ɾ������ͨ����ͬ���ģ������ǿ����ں�̨�߳���ִ��
    Async(EAsyncExecution::Thread, [this, SlotName, UserIndex]()
        {
            bool bSuccess = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);

            // �ص���Ϸ�̹߳㲥�¼�
            Async(EAsyncExecution::TaskGraphMainThread, [this, SlotName, bSuccess]()
                {
                    OnDeleteSaveComplete.Broadcast(SlotName, bSuccess);
                });
        });
}

// ========== ���ص����첽���� ==========

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

// ========== ��̬ί���ڲ�ʵ�� ==========

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

// ========== ���ٴ浵���� ==========

void USaveGameTool::QuickSave()
{
    FString QuickSaveSlot = TEXT("QuickSave_") + FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));

    // �������ٴ浵 - ʹ���µĽṹ�屣�淽��
    FPlayerSaveData PlayerData;
    // �����������һЩĬ�ϵĿ��ٴ浵����
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

    // �������µĿ��ٴ浵
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

    // �����Զ��浵 - ʹ�ýṹ�屣�淽��
    FPlayerSaveData PlayerData;
    // �����������һЩ�Զ��浵����
    if (SavePlayerDataStruct(PlayerData, AutoSaveSlot))
    {
        UE_LOG(LogTemp, Log, TEXT("AutoSave created: %s"), *AutoSaveSlot);

        // ����ɵ��Զ��浵���������5����
        CleanupOldSaves(5);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create AutoSave: %s"), *AutoSaveSlot);
    }
}

// ========== ���ݴ����ͻ�ȡ ==========

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

// ========== �ṹ�����ݲ��� ==========

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

// ========== ��ȫ�ļ��ط���������bool��ʾ�ɹ��� ==========

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

// ========== ֱ�ӽṹ�屣�� ==========

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

// ========== ��ݱ��淽�� ==========

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
        // Ӧ��������ݵ�Pawn
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

    // �������λ�ú���ת
    OutData.Location = PlayerPawn->GetActorLocation();
    OutData.Rotation = PlayerPawn->GetActorRotation();

    // ������������Ӹ����Pawn��ȡ���ݵ��߼�
    return true;
}

// ========== ��ǰ���ݹ��� ==========

USettingsSaveGame* USaveGameTool::GetCurrentSettings()
{
    if (!CurrentSettings)
    {
        // ���û�е�ǰ���ã����Լ��ػ򴴽�Ĭ������
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

// ========== �浵���� ==========

bool USaveGameTool::DoesSaveGameExist(const FString& SlotName, int32 UserIndex)
{
    return UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

TArray<FString> USaveGameTool::GetAllSaveSlots() const
{
    TArray<FString> SaveSlots;

    // ��ȡ�浵Ŀ¼
    FString SaveGameDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
    TArray<FString> SaveFiles;

    // ��������.sav�ļ�
    IFileManager::Get().FindFiles(SaveFiles, *SaveGameDir, TEXT(".sav"));

    for (const FString& SaveFile : SaveFiles)
    {
        // �Ƴ�.sav��չ��
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

    // �ռ����д浵��Ԫ����
    for (const FString& Slot : AllSaves)
    {
        FSaveGameMetadata Metadata = GetSaveMetadata(Slot);
        if (Metadata.SaveSlotName == Slot) // ��֤Ԫ������Ч
        {
            SaveMetadatas.Add(Metadata);
        }
    }

    // ��ʱ���������µ���ǰ��
    SaveMetadatas.Sort([](const FSaveGameMetadata& A, const FSaveGameMetadata& B)
        {
            return A.SaveDateTime > B.SaveDateTime;
        });

    // ɾ�������������Ƶľɴ浵
    for (int32 i = MaxSaveCount; i < SaveMetadatas.Num(); i++)
    {
        DeleteGameSync(SaveMetadatas[i].SaveSlotName);
        UE_LOG(LogTemp, Log, TEXT("Cleaned up old save: %s"), *SaveMetadatas[i].SaveSlotName);
    }
}

// ========== ���Թ��� ==========

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

    // ������֤
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

// ========== �ڲ�ʵ�� ==========

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

    // ʹ��UE�ı���ϵͳ
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

    // ������Ϸ�汾
    SaveGame->Metadata.GameVersion = TEXT("1.0.0");
    SaveGame->Metadata.SaveVersion = 1;
}

void USaveGameTool::HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("Async save completed: %s - %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failed"));

    // �㲥ί��
    OnSaveGameComplete.Broadcast(SlotName, bSuccess);

    // ִ����ͼ�ص�
    FOnSaveGameCallback* Callback = SaveCallbacks.Find(SlotName);
    if (Callback && Callback->IsBound())
    {
        Callback->Execute(SlotName, bSuccess);
        SaveCallbacks.Remove(SlotName);
    }

    // ִ�о�̬ί�лص�
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

    // �㲥ί��
    OnLoadGameComplete.Broadcast(SlotName, SaveGameBase, bSuccess);

    // ִ����ͼ�ص�
    FOnLoadGameCallback* Callback = LoadCallbacks.Find(SlotName);
    if (Callback && Callback->IsBound())
    {
        Callback->Execute(SlotName, SaveGameBase, bSuccess);
        LoadCallbacks.Remove(SlotName);
    }

    // ִ�о�̬ί�лص�
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