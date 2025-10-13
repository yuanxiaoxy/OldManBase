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

// �����ʼ��״̬
UENUM(BlueprintType)
enum class EWorldInitState : uint8
{
    NotInitialized UMETA(DisplayName = "Not Initialized"),
    Initializing   UMETA(DisplayName = "Initializing"),
    Initialized    UMETA(DisplayName = "Initialized"),
    Failed         UMETA(DisplayName = "Failed")
};

// �����������
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

// ��������
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

// �����ʼ�����ί��
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldInitialized, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldShutdown);

// ��ͼ�ص�ί��
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
    // ========== �����������ڹ��� ==========

    // ��ʼ�����磨����ڵ㣩
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorld();

    // �첽��ʼ������
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void InitializeWorldAsync();

    // �ر�����
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void ShutdownWorld();

    // ��������
    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    virtual void RestartWorld();

    // ========== ��ҹ��� ==========

    // �Ƿ�Ӧ��������ң�����д��
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    bool ShouldSpawnPlayer() const;
    virtual bool ShouldSpawnPlayer_Implementation() const;

    // ��ȡ����������ã�����д��
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FPlayerSpawnConfig GetPlayerSpawnConfig() const;
    virtual FPlayerSpawnConfig GetPlayerSpawnConfig_Implementation() const;

    // ������ң�����д��
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    APawn* SpawnPlayer(AController* NewPlayer);
    virtual APawn* SpawnPlayer_Implementation(AController* NewPlayer);

    // �Զ����������λ�ã�����д��
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Player")
    FTransform GetPlayerSpawnTransform(AController* PlayerController);
    virtual FTransform GetPlayerSpawnTransform_Implementation(AController* PlayerController);

    // ========== �������� ==========

    // ��ȡ�������ã�����д��
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameMode|Config")
    FWorldConfig GetWorldConfig() const;
    virtual FWorldConfig GetWorldConfig_Implementation() const;

    // ������������
    UFUNCTION(BlueprintCallable, Category = "GameMode|Config")
    void SetWorldConfig(const FWorldConfig& NewConfig);

    // ========== ��ܼ��� ==========

    // ��ȡ��Դ������
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UResourceManager* GetResourceManager() const;

    // ��ȡ��ʱ��������
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    UMonoManager* GetMonoManager() const;

    // ��ȡ�浵����
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|Framework")
    USaveGameTool* GetSaveGameTool() const;

    // ========== ��ʼ�����裨����д�� ==========

    // Ԥ��ʼ��������Ҫ��ʼ��֮ǰ��
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PreInitializeWorld();
    virtual void PreInitializeWorld_Implementation();

    // ��ʼ�����ϵͳ
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeFrameworkSystems();
    virtual void InitializeFrameworkSystems_Implementation();

    // ����������Դ
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void LoadWorldResources();
    virtual void LoadWorldResources_Implementation();

    // ��ʼ������״̬
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializeWorldState();
    virtual void InitializeWorldState_Implementation();

    // ��ʼ�����
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void InitializePlayers();
    virtual void InitializePlayers_Implementation();

    // ���ʼ��������Ҫ��ʼ��֮��
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Initialization")
    void PostInitializeWorld();
    virtual void PostInitializeWorld_Implementation();

    // ========== �رղ��裨����д�� ==========

    // Ԥ�ر�
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void PreShutdownWorld();
    virtual void PreShutdownWorld_Implementation();

    // ��������״̬
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void SaveWorldState();
    virtual void SaveWorldState_Implementation();

    // ������Դ
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void CleanupWorldResources();
    virtual void CleanupWorldResources_Implementation();

    // ��ر�
    UFUNCTION(BlueprintNativeEvent, Category = "GameMode|Shutdown")
    void PostShutdownWorld();
    virtual void PostShutdownWorld_Implementation();

    // ========== �¼�ί�� ==========

    UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
    FOnWorldInitialized OnWorldInitialized;

    UPROPERTY(BlueprintAssignable, Category = "GameMode|Events")
    FOnWorldShutdown OnWorldShutdown;

    // ========== ���ص����첽��ʼ�� ==========

    UFUNCTION(BlueprintCallable, Category = "GameMode|World")
    void InitializeWorldWithCallback(const FOnWorldInitCallback& Callback);

    // ========== ״̬��ѯ ==========

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    EWorldInitState GetWorldInitState() const { return WorldInitState; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    bool IsWorldInitialized() const { return WorldInitState == EWorldInitState::Initialized; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameMode|World")
    bool IsWorldInitializing() const { return WorldInitState == EWorldInitState::Initializing; }

protected:
    // ========== �ڲ�ʵ�� ==========

    // �ڲ���ʼ����ɴ��� - �Ƴ�UFUNCTION����Ϊ����Ҫ��ͼ����
    void HandleWorldInitialized(bool bSuccess);

    // �첽��ʼ����ʱ���ص� - �Ƴ�UFUNCTION
    void ExecuteAsyncInitialization();

    // ����Ĭ�����
    APawn* SpawnDefaultPlayer(AController* NewPlayer);

    // ��ȡ������ɵ�
    FTransform GetRandomSpawnTransform() const;

private:
    // �����ʼ��״̬
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State", meta = (AllowPrivateAccess = "true"))
    EWorldInitState WorldInitState;

    // ��������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Config", meta = (AllowPrivateAccess = "true"))
    FWorldConfig WorldConfiguration;

    // �����������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Player", meta = (AllowPrivateAccess = "true"))
    FPlayerSpawnConfig PlayerSpawnConfiguration;

    // �ص��洢
    TArray<FOnWorldInitCallback> InitCallbacks;

    // �첽��ʼ����ʱ��ID
    FString AsyncInitTimerId;
};