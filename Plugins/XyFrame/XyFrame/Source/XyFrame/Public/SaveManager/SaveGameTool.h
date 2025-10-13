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

// �浵������
UENUM(BlueprintType)
enum class ESaveSlotType : uint8
{
    AutoSave     UMETA(DisplayName = "Auto Save"),
    ManualSave   UMETA(DisplayName = "Manual Save"),
    QuickSave    UMETA(DisplayName = "Quick Save"),
    Checkpoint   UMETA(DisplayName = "Checkpoint"),
    System       UMETA(DisplayName = "System Data")
};

// �浵����״̬
UENUM(BlueprintType)
enum class ESaveDataState : uint8
{
    Valid        UMETA(DisplayName = "Valid"),
    Corrupted    UMETA(DisplayName = "Corrupted"),
    Outdated     UMETA(DisplayName = "Outdated"),
    Loading      UMETA(DisplayName = "Loading"),
    Saving       UMETA(DisplayName = "Saving")
};

// �浵Ԫ����
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

// �����浵��
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

// ��Ҵ浵��
UCLASS(Blueprintable)
class UPlayerSaveGame : public USaveGameBase
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player")
    FPlayerSaveData PlayerData;

    // ��ݷ���
    UFUNCTION(BlueprintCallable, Category = "Player")
    void SetPlayerData(const FPlayerSaveData& NewData) { PlayerData = NewData; }

    UFUNCTION(BlueprintCallable, Category = "Player")
    FPlayerSaveData GetPlayerData() const { return PlayerData; }
};

// ����浵��
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

// ���ô浵��
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

// ���ȴ浵��
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

// �浵���ί��
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameComplete, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoadGameComplete, const FString&, SlotName, USaveGameBase*, SaveGame, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeleteSaveComplete, const FString&, SlotName, bool, bSuccess);

// ��ͼ�ص�ί��
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnSaveGameCallback, const FString&, SlotName, bool, bSuccess);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnLoadGameCallback, const FString&, SlotName, USaveGameBase*, SaveGame, bool, bSuccess);

// ��̬ί�� - ����C++�ص�
DECLARE_DELEGATE_TwoParams(FOnSaveGameStaticDelegate, const FString&, bool);
DECLARE_DELEGATE_ThreeParams(FOnLoadGameStaticDelegate, const FString&, USaveGameBase*, bool);

// ���ؽ���ṹ��
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

    // ��������
    DECLARE_SINGLETON(USaveGameTool)

public:
    // ���캯������������
    USaveGameTool();
    virtual ~USaveGameTool() override;

    // ��ʼ���浵������
    UFUNCTION(BlueprintCallable, Category = "SaveGame")
    void InitializeSaveTool();

    // ������ʼ������
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // ��ȡ�����Ŀɷ��ʷ���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveGame", meta = (DisplayName = "Get Save Game Tool"))
    static USaveGameTool* GetSaveGameTool() { return GetInstance(); }

    // ========== �����浵���� ==========

    // ͬ��������Ϸ
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    bool SaveGameSync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex = 0);

    // ͬ��������Ϸ
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    USaveGameBase* LoadGameSync(const FString& SlotName, int32 UserIndex = 0);

    // ͬ��ɾ���浵
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Sync")
    bool DeleteGameSync(const FString& SlotName, int32 UserIndex = 0);

    // ========== �첽�浵���� ==========

    // �첽������Ϸ
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void SaveGameAsync(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex = 0);

    // �첽������Ϸ
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void LoadGameAsync(const FString& SlotName, int32 UserIndex = 0);

    // �첽ɾ���浵
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void DeleteGameAsync(const FString& SlotName, int32 UserIndex = 0);

    // ========== ���ص����첽���� ==========

    // �첽������Ϸ����ͼ�ص���
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameCallback& Callback, int32 UserIndex = 0);

    // �첽������Ϸ����ͼ�ص���
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Async")
    void LoadGameAsyncWithCallback(const FString& SlotName, const FOnLoadGameCallback& Callback, int32 UserIndex = 0);

    // ========== C++��̬ί�лص����� ==========

    // �첽������Ϸ��C++��̬ί�У�
    template<typename T>
    void SaveGameAsyncWithCallback(const FString& SlotName, USaveGameBase* SaveGameObject, T* Object, void(T::* Function)(const FString&, bool), int32 UserIndex = 0)
    {
        FOnSaveGameStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalSaveGameAsyncWithStaticCallback(SlotName, SaveGameObject, StaticDelegate, UserIndex);
    }

    // �첽������Ϸ���޲���C++��̬ί�У�
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

    // �첽������Ϸ��C++��̬ί�У�
    template<typename T>
    void LoadGameAsyncWithCallback(const FString& SlotName, T* Object, void(T::* Function)(const FString&, USaveGameBase*, bool), int32 UserIndex = 0)
    {
        FOnLoadGameStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadGameAsyncWithStaticCallback(SlotName, StaticDelegate, UserIndex);
    }

    // �첽������Ϸ���޲���C++��̬ί�У�
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

    // ========== ���ٴ浵���� ==========

    // ���ٱ���
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Quick")
    void QuickSave();

    // ���ټ���
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Quick")
    bool QuickLoad();

    // �Զ�����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Auto")
    void AutoSave();

    // ========== ���ݴ����ͻ�ȡ ==========

    // �����յĴ浵����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UPlayerSaveGame* CreatePlayerSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UWorldSaveGame* CreateWorldSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* CreateSettingsSaveGame();

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UProgressSaveGame* CreateProgressSaveGame();

    // �ӽṹ�崴���浵����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UPlayerSaveGame* CreatePlayerSaveGameFromData(const FPlayerSaveData& PlayerData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UWorldSaveGame* CreateWorldSaveGameFromData(const FWorldSaveData& WorldData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* CreateSettingsSaveGameFromData(const FSettingsData& SettingsData);

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    UProgressSaveGame* CreateProgressSaveGameFromData(const FProgressData& ProgressData);

    // �Ӵ浵�����ȡ�ṹ������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FPlayerSaveData GetPlayerDataFromSaveGame(UPlayerSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FWorldSaveData GetWorldDataFromSaveGame(UWorldSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FSettingsData GetSettingsDataFromSaveGame(USettingsSaveGame* SaveGame) const;

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FProgressData GetProgressDataFromSaveGame(UProgressSaveGame* SaveGame) const;

    // ========== ��ȫ�ļ��ط���������bool��ʾ�ɹ��� ==========

    // ��ȫ����������ݵ��ṹ��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadPlayerDataStruct(FPlayerSaveData& OutData, const FString& SlotName = TEXT("PlayerData"));

    // ��ȫ�����������ݵ��ṹ��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadWorldDataStruct(FWorldSaveData& OutData, const FString& SlotName = TEXT("WorldData"));

    // ��ȫ�����������ݵ��ṹ��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadSettingsDataStruct(FSettingsData& OutData, const FString& SlotName = TEXT("GameSettings"));

    // ��ȫ���ؽ������ݵ��ṹ��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadProgressDataStruct(FProgressData& OutData, const FString& SlotName = TEXT("ProgressData"));

    // ========== ֱ�ӽṹ�屣�� ==========

    // ֱ��ʹ�ýṹ�屣������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SavePlayerDataStruct(const FPlayerSaveData& PlayerData, const FString& SlotName = TEXT("PlayerData"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveWorldDataStruct(const FWorldSaveData& WorldData, const FString& SlotName = TEXT("WorldData"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveSettingsDataStruct(const FSettingsData& SettingsData, const FString& SlotName = TEXT("GameSettings"));

    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveProgressDataStruct(const FProgressData& ProgressData, const FString& SlotName = TEXT("ProgressData"));

    // ========== ��ݱ��淽�� ==========

    // ����������ݣ���Pawn�������ݣ�
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SavePlayerData(APawn* PlayerPawn, const FString& SlotName = TEXT("PlayerData"));

    // ����������ݵ�Pawn
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadPlayerData(APawn* PlayerPawn, const FString& SlotName = TEXT("PlayerData"));

    // ��Pawn�����������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool CreatePlayerDataFromPawn(APawn* PlayerPawn, FPlayerSaveData& OutData);

    // ========== ��ǰ���ݹ��� ==========

    // ��ȡ��ǰ���ã����û������ػ򴴽�Ĭ�����ã�
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    USettingsSaveGame* GetCurrentSettings();

    // ���õ�ǰ����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    void SetCurrentSettings(USettingsSaveGame* NewSettings);

    // ��ȡ��ǰ��������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    FSettingsData GetCurrentSettingsData() const;

    // ���õ�ǰ��������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Data")
    void SetCurrentSettingsData(const FSettingsData& NewData);

    // ���浱ǰ����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool SaveCurrentSettings(const FString& SlotName = TEXT("GameSettings"));

    // ��������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Convenience")
    bool LoadSettings(const FString& SlotName = TEXT("GameSettings"));

    // ========== �浵���� ==========

    // ���浵�Ƿ����
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    bool DoesSaveGameExist(const FString& SlotName, int32 UserIndex = 0);

    // ��ȡ���д浵��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    TArray<FString> GetAllSaveSlots() const;

    // ��ȡ�浵Ԫ���ݣ������������浵��
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    FSaveGameMetadata GetSaveMetadata(const FString& SlotName, int32 UserIndex = 0);

    // ��ȡ�浵��С���ֽڣ�
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    int64 GetSaveGameSize(const FString& SlotName, int32 UserIndex = 0) const;

    // ��ȡ�ܴ浵��С
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    int64 GetTotalSaveSize() const;

    // ����ɴ浵
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Management")
    void CleanupOldSaves(int32 MaxSaveCount = 10);

    // ========== ���Թ��� ==========

    // ��ӡ���д浵
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    void PrintAllSaves();

    // ��֤�浵������
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    bool ValidateSaveGame(const FString& SlotName, int32 UserIndex = 0);

    // ��ӡ�浵ͳ����Ϣ
    UFUNCTION(BlueprintCallable, Category = "SaveGame|Debug")
    void PrintSaveStatistics();

    // ========== �¼�ί�� ==========

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnSaveGameComplete OnSaveGameComplete;

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnLoadGameComplete OnLoadGameComplete;

    UPROPERTY(BlueprintAssignable, Category = "SaveGame|Events")
    FOnDeleteSaveComplete OnDeleteSaveComplete;

private:
    // ��ǰ����
    UPROPERTY()
    USettingsSaveGame* CurrentSettings;

    // ��ǰ�������ݻ���
    FSettingsData CurrentSettingsData;

    // �ص�ӳ��
    TMap<FString, FOnSaveGameCallback> SaveCallbacks;
    TMap<FString, FOnLoadGameCallback> LoadCallbacks;

    // ��̬ί��ӳ��
    TMap<FString, FOnSaveGameStaticDelegate> SaveStaticCallbacks;
    TMap<FString, FOnLoadGameStaticDelegate> LoadStaticCallbacks;

    // �ڲ�ʵ�ַ���
    FString GenerateBackupName(const FString& SlotName) const;
    bool InternalSaveGame(const FString& SlotName, USaveGameBase* SaveGameObject, int32 UserIndex);
    USaveGameBase* InternalLoadGame(const FString& SlotName, int32 UserIndex);
    void UpdateSaveMetadata(USaveGameBase* SaveGame, const FString& SlotName, ESaveSlotType SlotType);

    // ��̬ί���ڲ�����
    void InternalSaveGameAsyncWithStaticCallback(const FString& SlotName, USaveGameBase* SaveGameObject, const FOnSaveGameStaticDelegate& Callback, int32 UserIndex);
    void InternalLoadGameAsyncWithStaticCallback(const FString& SlotName, const FOnLoadGameStaticDelegate& Callback, int32 UserIndex);

    // �첽������ɴ���
    UFUNCTION()
    void HandleAsyncSaveComplete(const FString& SlotName, const int32 UserIndex, bool bSuccess);

    UFUNCTION()
    void HandleAsyncLoadComplete(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame);

    // ��ȡ����
    UWorld* GetWorld() const override;
};