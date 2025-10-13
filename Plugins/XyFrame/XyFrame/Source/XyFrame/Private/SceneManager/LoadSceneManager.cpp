// Fill out your copyright notice in the Description page of Project Settings.

#include "SceneManager/LoadSceneManager.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"

// ��̬ʵ������
template<>
ULoadSceneManager* TSingleton<ULoadSceneManager>::SingletonInstance = nullptr;

ULoadSceneManager::ULoadSceneManager()
{
    // ���캯��
}

ULoadSceneManager::~ULoadSceneManager()
{
    // ֹͣ���м�ʱ��
    for (auto& TimerPair : ProgressTimerHandles)
    {
        StopProgressTimer(TimerPair.Key);
    }
    ProgressTimerHandles.Empty();
}

void ULoadSceneManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("LoadSceneManager InitializeSingleton called"));
    InitializeSceneManager();
}

void ULoadSceneManager::InitializeSceneManager()
{
    UE_LOG(LogTemp, Log, TEXT("Scene Manager Initialized"));

    // ɨ����õ�ͼ�ļ�
    ScanForMapFiles();
}

// ========== ͬ���������� ==========

void ULoadSceneManager::LoadSceneByIndex(int32 SceneIndex, ESceneLoadMode Mode)
{
    if (!IsValidSceneIndex(SceneIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid scene index: %d"), SceneIndex);
        return;
    }

    FString SceneName = GetSceneNameFromIndex(SceneIndex);
    LoadSceneByName(SceneName, Mode);
}

void ULoadSceneManager::LoadSceneByName(const FString& SceneName, ESceneLoadMode Mode)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load scene without valid world"));
        return;
    }

    // ֱ��ʹ�ó������Ƽ��أ������Ѿ��� Build Settings �У�
    if (Mode == ESceneLoadMode::Single)
    {
        UGameplayStatics::OpenLevel(World, FName(*SceneName));
        UE_LOG(LogTemp, Log, TEXT("Opening level: %s"), *SceneName);
    }
    else
    {
        // ���Ӽ���ģʽ
        UGameplayStatics::LoadStreamLevel(World, FName(*SceneName), true, true, FLatentActionInfo());
        UE_LOG(LogTemp, Log, TEXT("Loading streaming level: %s"), *SceneName);
    }
}

void ULoadSceneManager::LoadSceneByPath(const FString& MapPath, ESceneLoadMode Mode)
{
    FString SanitizedPath = SanitizeMapPath(MapPath);

    if (!DoesMapFileExist(SanitizedPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Map file does not exist: %s"), *SanitizedPath);
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load scene without valid world"));
        return;
    }

    // ��ȡ��ͼ����
    FString MapName = ExtractMapNameFromPath(SanitizedPath);

    if (Mode == ESceneLoadMode::Single)
    {
        UGameplayStatics::OpenLevel(World, FName(*MapName));
    }
    else
    {
        // ���Ӽ���ģʽ
        UGameplayStatics::LoadStreamLevel(World, FName(*MapName), true, true, FLatentActionInfo());
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded scene by path: %s -> %s in %s mode"), *SanitizedPath, *MapName, *UEnum::GetValueAsString(Mode));
}

void ULoadSceneManager::LoadSceneByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode)
{
    if (!IsMapAssetValid(MapAsset))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid map asset: %s"), *MapAsset.ToString());
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load scene without valid world"));
        return;
    }

    // ����1��ʹ�õ�ͼ�ʲ������ƣ������� Build Settings �У�
    FString MapName = GetMapAssetDisplayName(MapAsset);

    if (Mode == ESceneLoadMode::Single)
    {
        UGameplayStatics::OpenLevel(World, FName(*MapName));
        UE_LOG(LogTemp, Log, TEXT("Opening level by asset: %s"), *MapName);
    }
    else
    {
        UGameplayStatics::LoadStreamLevel(World, FName(*MapName), true, true, FLatentActionInfo());
        UE_LOG(LogTemp, Log, TEXT("Loading streaming level by asset: %s"), *MapName);
    }
}

// ========== ʹ�������ʲ�·������ ==========

void ULoadSceneManager::LoadSceneByFullPath(const FString& FullMapPath, ESceneLoadMode Mode)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot load scene without valid world"));
        return;
    }

    // ������·������ȡ��ͼ����
    FString MapName = ExtractMapNameFromFullPath(FullMapPath);

    if (Mode == ESceneLoadMode::Single)
    {
        UGameplayStatics::OpenLevel(World, FName(*MapName));
        UE_LOG(LogTemp, Log, TEXT("Opening level by full path: %s -> %s"), *FullMapPath, *MapName);
    }
    else
    {
        UGameplayStatics::LoadStreamLevel(World, FName(*MapName), true, true, FLatentActionInfo());
        UE_LOG(LogTemp, Log, TEXT("Loading streaming level by full path: %s -> %s"), *FullMapPath, *MapName);
    }
}

void ULoadSceneManager::ReloadCurrentScene()
{
    FString CurrentScene = GetCurrentSceneName();
    if (!CurrentScene.IsEmpty())
    {
        LoadSceneByName(CurrentScene, ESceneLoadMode::Single);
    }
}

void ULoadSceneManager::LoadNextScene(bool bCyclical)
{
    int32 CurrentIndex = GetCurrentSceneIndex();
    int32 NextIndex = CurrentIndex + 1;

    if (NextIndex >= GetSceneCount())
    {
        if (bCyclical)
        {
            NextIndex = 0;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No next scene available"));
            return;
        }
    }

    LoadSceneByIndex(NextIndex, ESceneLoadMode::Single);
}

void ULoadSceneManager::LoadPreviousScene(bool bCyclical)
{
    int32 CurrentIndex = GetCurrentSceneIndex();
    int32 PreviousIndex = CurrentIndex - 1;

    if (PreviousIndex < 0)
    {
        if (bCyclical)
        {
            PreviousIndex = GetSceneCount() - 1;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No previous scene available"));
            return;
        }
    }

    LoadSceneByIndex(PreviousIndex, ESceneLoadMode::Single);
}

// ========== �첽�������� ==========

FString ULoadSceneManager::LoadSceneAsyncByIndex(int32 SceneIndex, ESceneLoadMode Mode, bool bActivateAfterLoad)
{
    if (!IsValidSceneIndex(SceneIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid scene index: %d"), SceneIndex);
        return FString();
    }

    FString SceneName = GetSceneNameFromIndex(SceneIndex);
    return LoadSceneAsyncByName(SceneName, Mode, bActivateAfterLoad);
}

FString ULoadSceneManager::LoadSceneAsyncByName(const FString& SceneName, ESceneLoadMode Mode, bool bActivateAfterLoad)
{
    if (!DoesSceneExist(SceneName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Scene does not exist: %s"), *SceneName);
        return FString();
    }

    FString RequestId = GenerateRequestId();

    FSceneAsyncLoadRequest NewRequest;
    NewRequest.RequestId = RequestId;
    NewRequest.SceneName = SceneName;
    NewRequest.LoadState = ESceneLoadState::Loading;
    NewRequest.Progress = 0.0f;

    AsyncRequests.Add(RequestId, NewRequest);

    // ʹ��UGameplayStatics���첽����
    UWorld* World = GetWorld();
    if (World)
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget = this;
        LatentInfo.ExecutionFunction = FName("OnAsyncLoadComplete");
        LatentInfo.Linkage = 0;
        LatentInfo.UUID = GetTypeHash(RequestId);

        // �洢UUID�Ա��������
        AsyncRequests[RequestId].LatentActionUUID = LatentInfo.UUID;

        if (Mode == ESceneLoadMode::Single)
        {
            // ���ڵ���ģʽ��ʹ��OpenLevel
            UGameplayStatics::OpenLevel(World, FName(*SceneName));
            // �������
            CompleteAsyncRequest(RequestId, true);
        }
        else
        {
            // ���ڸ���ģʽ��ʹ�����ͼ���
            UGameplayStatics::LoadStreamLevel(World, FName(*SceneName), true, bActivateAfterLoad, LatentInfo);
            // �������ȸ��¼�ʱ��
            StartProgressTimer(RequestId);
        }
    }

    return RequestId;
}

FString ULoadSceneManager::LoadSceneAsyncByPath(const FString& MapPath, ESceneLoadMode Mode, bool bActivateAfterLoad)
{
    FString SanitizedPath = SanitizeMapPath(MapPath);

    if (!DoesMapFileExist(SanitizedPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Map file does not exist: %s"), *SanitizedPath);
        return FString();
    }

    FString MapName = ExtractMapNameFromPath(SanitizedPath);
    return LoadSceneAsyncByName(MapName, Mode, bActivateAfterLoad);
}

FString ULoadSceneManager::LoadSceneAsyncByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode, bool bActivateAfterLoad)
{
    if (!IsMapAssetValid(MapAsset))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid map asset: %s"), *MapAsset.ToString());
        return FString();
    }

    FString RequestId = GenerateRequestId();

    FSceneAsyncLoadRequest NewRequest;
    NewRequest.RequestId = RequestId;
    NewRequest.SceneName = GetMapAssetDisplayName(MapAsset);
    NewRequest.LoadState = ESceneLoadState::Loading;
    NewRequest.Progress = 0.0f;

    AsyncRequests.Add(RequestId, NewRequest);

    UWorld* World = GetWorld();
    if (World)
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget = this;
        LatentInfo.ExecutionFunction = FName("OnAsyncLoadComplete");
        LatentInfo.Linkage = 0;
        LatentInfo.UUID = GetTypeHash(RequestId);

        // �洢UUID�Ա��������
        AsyncRequests[RequestId].LatentActionUUID = LatentInfo.UUID;

        if (Mode == ESceneLoadMode::Single)
        {
            // ���ڵ���ģʽ��ʹ��OpenLevel
            UGameplayStatics::OpenLevel(World, FName(*NewRequest.SceneName));
            // �������
            CompleteAsyncRequest(RequestId, true);
        }
        else
        {
            // ���ڸ���ģʽ��ʹ�����ͼ���
            UGameplayStatics::LoadStreamLevel(World, FName(*NewRequest.SceneName), true, bActivateAfterLoad, LatentInfo);
            // �������ȸ��¼�ʱ��
            StartProgressTimer(RequestId);
        }
    }

    return RequestId;
}

FString ULoadSceneManager::LoadSceneAsyncByFullPath(const FString& FullMapPath, ESceneLoadMode Mode, bool bActivateAfterLoad)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FString();
    }

    FString RequestId = GenerateRequestId();
    FString MapName = ExtractMapNameFromFullPath(FullMapPath);

    FSceneAsyncLoadRequest NewRequest;
    NewRequest.RequestId = RequestId;
    NewRequest.SceneName = MapName;
    NewRequest.LoadState = ESceneLoadState::Loading;
    NewRequest.Progress = 0.0f;

    AsyncRequests.Add(RequestId, NewRequest);

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = FName("OnAsyncLoadComplete");
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = GetTypeHash(RequestId);

    // �洢UUID�Ա��������
    AsyncRequests[RequestId].LatentActionUUID = LatentInfo.UUID;

    if (Mode == ESceneLoadMode::Single)
    {
        UGameplayStatics::OpenLevel(World, FName(*MapName));
        CompleteAsyncRequest(RequestId, true);
    }
    else
    {
        UGameplayStatics::LoadStreamLevel(World, FName(*MapName), true, bActivateAfterLoad, LatentInfo);
        StartProgressTimer(RequestId);
    }

    return RequestId;
}

FString ULoadSceneManager::ReloadCurrentSceneAsync(bool bActivateAfterLoad)
{
    FString CurrentScene = GetCurrentSceneName();
    if (CurrentScene.IsEmpty())
    {
        return FString();
    }
    return LoadSceneAsyncByName(CurrentScene, ESceneLoadMode::Single, bActivateAfterLoad);
}

FString ULoadSceneManager::LoadNextSceneAsync(bool bCyclical, bool bActivateAfterLoad)
{
    int32 CurrentIndex = GetCurrentSceneIndex();
    int32 NextIndex = CurrentIndex + 1;

    if (NextIndex >= GetSceneCount())
    {
        if (bCyclical)
        {
            NextIndex = 0;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No next scene available"));
            return FString();
        }
    }

    return LoadSceneAsyncByIndex(NextIndex, ESceneLoadMode::Single, bActivateAfterLoad);
}

FString ULoadSceneManager::LoadPreviousSceneAsync(bool bCyclical, bool bActivateAfterLoad)
{
    int32 CurrentIndex = GetCurrentSceneIndex();
    int32 PreviousIndex = CurrentIndex - 1;

    if (PreviousIndex < 0)
    {
        if (bCyclical)
        {
            PreviousIndex = GetSceneCount() - 1;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No previous scene available"));
            return FString();
        }
    }

    return LoadSceneAsyncByIndex(PreviousIndex, ESceneLoadMode::Single, bActivateAfterLoad);
}

// ========== �첽�������أ����ص��� ==========

void ULoadSceneManager::LoadSceneAsyncWithCallback(int32 SceneIndex, ESceneLoadMode Mode, bool bActivateAfterLoad,
    const FOnSceneLoadedCallback& OnComplete)
{
    FString RequestId = LoadSceneAsyncByIndex(SceneIndex, Mode, bActivateAfterLoad);

    if (!RequestId.IsEmpty() && OnComplete.IsBound())
    {
        LoadCallbacks.Add(RequestId, OnComplete);
    }
}

void ULoadSceneManager::LoadSceneAsyncByPathWithCallback(const FString& MapPath, ESceneLoadMode Mode, bool bActivateAfterLoad,
    const FOnSceneLoadedCallback& OnComplete)
{
    FString RequestId = LoadSceneAsyncByPath(MapPath, Mode, bActivateAfterLoad);

    if (!RequestId.IsEmpty() && OnComplete.IsBound())
    {
        LoadCallbacks.Add(RequestId, OnComplete);
    }
}

void ULoadSceneManager::LoadSceneAsyncByAssetWithCallback(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode, bool bActivateAfterLoad,
    const FOnSceneLoadedCallback& OnComplete)
{
    if (!IsMapAssetValid(MapAsset))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid map asset: %s"), *MapAsset.ToString());
        return;
    }

    FString MapPath = MapAsset.ToSoftObjectPath().GetAssetPathString();
    FString RequestId = LoadSceneAsyncByPath(MapPath, Mode, bActivateAfterLoad);

    if (!RequestId.IsEmpty() && OnComplete.IsBound())
    {
        LoadCallbacks.Add(RequestId, OnComplete);
    }
}

// ========== ����ж�� ==========

FString ULoadSceneManager::UnloadSceneAsync(const FString& SceneName)
{
    FString RequestId = GenerateRequestId();

    FSceneAsyncLoadRequest NewRequest;
    NewRequest.RequestId = RequestId;
    NewRequest.SceneName = SceneName;
    NewRequest.LoadState = ESceneLoadState::Loading;
    NewRequest.Progress = 0.0f;

    AsyncRequests.Add(RequestId, NewRequest);

    UWorld* World = GetWorld();
    if (World)
    {
        FLatentActionInfo LatentInfo;
        LatentInfo.CallbackTarget = this;
        LatentInfo.ExecutionFunction = FName("OnAsyncUnloadComplete");
        LatentInfo.Linkage = 0;
        LatentInfo.UUID = GetTypeHash(RequestId);

        // �洢UUID�Ա��������
        AsyncRequests[RequestId].LatentActionUUID = LatentInfo.UUID;

        UGameplayStatics::UnloadStreamLevel(World, FName(*SceneName), LatentInfo, true);

        // �������ȸ��¼�ʱ��
        StartProgressTimer(RequestId);
    }

    return RequestId;
}

void ULoadSceneManager::UnloadAllAdditiveScenes()
{
    TArray<FString> LoadedScenes = GetAllLoadedScenes();
    FString CurrentScene = GetCurrentSceneName();

    for (const FString& SceneName : LoadedScenes)
    {
        if (SceneName != CurrentScene)
        {
            UnloadSceneAsync(SceneName);
        }
    }
}

// ========== ��ͼ���͹��� ==========

void ULoadSceneManager::LoadStreamingLevel(const FString& LevelName, bool bMakeVisibleAfterLoad, bool bShouldBlockOnLoad)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ULevelStreaming* StreamingLevel = GetStreamingLevelByName(LevelName);
    if (StreamingLevel)
    {
        StreamingLevel->SetShouldBeLoaded(true);
        StreamingLevel->SetShouldBeVisible(bMakeVisibleAfterLoad);

        if (bShouldBlockOnLoad)
        {
            StreamingLevel->bShouldBlockOnLoad = true;
        }

        // �󶨼������ί��
        StreamingLevel->OnLevelLoaded.AddDynamic(this, &ULoadSceneManager::OnStreamingLevelLoaded);

        UE_LOG(LogTemp, Log, TEXT("Loading streaming level: %s"), *LevelName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Streaming level not found: %s"), *LevelName);
    }
}

void ULoadSceneManager::LoadStreamingLevelByPath(const FString& LevelPath, bool bMakeVisibleAfterLoad, bool bShouldBlockOnLoad)
{
    FString SanitizedPath = SanitizeMapPath(LevelPath);
    FString LevelName = ExtractMapNameFromPath(SanitizedPath);
    LoadStreamingLevel(LevelName, bMakeVisibleAfterLoad, bShouldBlockOnLoad);
}

void ULoadSceneManager::LoadStreamingLevelByAsset(const TSoftObjectPtr<UWorld>& MapAsset, bool bMakeVisibleAfterLoad, bool bShouldBlockOnLoad)
{
    if (!IsMapAssetValid(MapAsset))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid map asset: %s"), *MapAsset.ToString());
        return;
    }

    FString MapPath = MapAsset.ToSoftObjectPath().GetAssetPathString();
    LoadStreamingLevelByPath(MapPath, bMakeVisibleAfterLoad, bShouldBlockOnLoad);
}

void ULoadSceneManager::UnloadStreamingLevel(const FString& LevelName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ULevelStreaming* StreamingLevel = GetStreamingLevelByName(LevelName);
    if (StreamingLevel)
    {
        StreamingLevel->SetShouldBeLoaded(false);
        StreamingLevel->SetShouldBeVisible(false);

        // ��ж�����ί��
        StreamingLevel->OnLevelUnloaded.AddDynamic(this, &ULoadSceneManager::OnStreamingLevelUnloaded);

        UE_LOG(LogTemp, Log, TEXT("Unloading streaming level: %s"), *LevelName);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Streaming level not found: %s"), *LevelName);
    }
}

void ULoadSceneManager::SetStreamingLevelVisible(const FString& LevelName, bool bVisible)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ULevelStreaming* StreamingLevel = GetStreamingLevelByName(LevelName);
    if (StreamingLevel && StreamingLevel->IsLevelLoaded())
    {
        StreamingLevel->SetShouldBeVisible(bVisible);

        UE_LOG(LogTemp, Log, TEXT("Set streaming level %s visibility: %s"), *LevelName, bVisible ? TEXT("Visible") : TEXT("Hidden"));
    }
}

TArray<FStreamingLevelInfo> ULoadSceneManager::GetAllStreamingLevelsInfo() const
{
    TArray<FStreamingLevelInfo> LevelsInfo;

    UWorld* World = GetWorld();
    if (!World)
    {
        return LevelsInfo;
    }

    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (StreamingLevel)
        {
            FStreamingLevelInfo LevelInfo;
            LevelInfo.LevelName = StreamingLevel->GetWorldAssetPackageName();
            LevelInfo.PackageName = StreamingLevel->PackageNameToLoad.ToString();
            LevelInfo.bIsLoaded = StreamingLevel->IsLevelLoaded();
            LevelInfo.bShouldBeLoaded = StreamingLevel->ShouldBeLoaded();
            LevelInfo.bShouldBeVisible = StreamingLevel->ShouldBeVisible();

            LevelsInfo.Add(LevelInfo);
        }
    }

    return LevelsInfo;
}

bool ULoadSceneManager::IsStreamingLevelLoaded(const FString& LevelName) const
{
    ULevelStreaming* StreamingLevel = GetStreamingLevelByName(LevelName);
    return StreamingLevel && StreamingLevel->IsLevelLoaded();
}

void ULoadSceneManager::LoadStreamingLevels(const TArray<FString>& LevelNames, bool bMakeVisibleAfterLoad)
{
    for (const FString& LevelName : LevelNames)
    {
        LoadStreamingLevel(LevelName, bMakeVisibleAfterLoad, false);
    }
}

void ULoadSceneManager::UnloadStreamingLevels(const TArray<FString>& LevelNames)
{
    for (const FString& LevelName : LevelNames)
    {
        UnloadStreamingLevel(LevelName);
    }
}

// ========== ��ͼ�ļ����� ==========

TArray<FMapFileInfo> ULoadSceneManager::GetAllAvailableMaps() const
{
    return AvailableMaps;
}

TArray<FMapFileInfo> ULoadSceneManager::FindMapsInFolder(const FString& FolderPath) const
{
    TArray<FMapFileInfo> FoundMaps;
    FString SanitizedPath = SanitizeMapPath(FolderPath);

    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        if (MapInfo.MapPath.StartsWith(SanitizedPath))
        {
            FoundMaps.Add(MapInfo);
        }
    }

    return FoundMaps;
}

bool ULoadSceneManager::DoesMapFileExist(const FString& MapPath) const
{
    FString SanitizedPath = SanitizeMapPath(MapPath);

    // ����Ƿ��ڿ��õ�ͼ�б���
    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        if (MapInfo.MapPath == SanitizedPath)
        {
            return true;
        }
    }

    return false;
}

FString ULoadSceneManager::GetMapDisplayName(const FString& MapPath) const
{
    FString SanitizedPath = SanitizeMapPath(MapPath);

    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        if (MapInfo.MapPath == SanitizedPath)
        {
            return MapInfo.DisplayName;
        }
    }

    return FString();
}

// ========== ������Ϣ��ѯ ==========

FString ULoadSceneManager::GetCurrentSceneName() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FString();
    }
    return World->GetMapName();
}

FString ULoadSceneManager::GetCurrentScenePath() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FString();
    }
    return World->GetOutermost()->GetName();
}

int32 ULoadSceneManager::GetCurrentSceneIndex() const
{
    FString CurrentScene = GetCurrentSceneName();
    return GetSceneIndexFromName(CurrentScene);
}

int32 ULoadSceneManager::GetSceneCount() const
{
    // ���ؿ��õ�ͼ����
    return AvailableMaps.Num();
}

FSceneInfo ULoadSceneManager::GetSceneInfo(int32 SceneIndex) const
{
    FSceneInfo Info;

    if (IsValidSceneIndex(SceneIndex))
    {
        Info.SceneName = GetSceneNameFromIndex(SceneIndex);
        Info.BuildIndex = SceneIndex;

        // ���Ҷ�Ӧ�ĵ�ͼ��Ϣ
        for (const FMapFileInfo& MapInfo : AvailableMaps)
        {
            if (MapInfo.MapName == Info.SceneName)
            {
                Info.ScenePath = MapInfo.MapPath;
                Info.LoadState = ESceneLoadState::Loaded; // �򻯴���
                break;
            }
        }
    }

    return Info;
}

bool ULoadSceneManager::IsSceneIndexValid(int32 SceneIndex) const
{
    return SceneIndex >= 0 && SceneIndex < GetSceneCount();
}

bool ULoadSceneManager::DoesSceneExist(const FString& SceneName) const
{
    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        if (MapInfo.MapName == SceneName)
        {
            return true;
        }
    }
    return false;
}

TArray<FString> ULoadSceneManager::GetAllLoadedScenes() const
{
    TArray<FString> LoadedScenes;

    UWorld* World = GetWorld();
    if (!World)
    {
        return LoadedScenes;
    }

    // ���������
    LoadedScenes.Add(GetCurrentSceneName());

    // ������ͳ���
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (StreamingLevel && StreamingLevel->IsLevelLoaded())
        {
            LoadedScenes.Add(StreamingLevel->GetWorldAssetPackageName());
        }
    }

    return LoadedScenes;
}

TArray<AActor*> ULoadSceneManager::GetRootActorsInScene(const FString& SceneName) const
{
    TArray<AActor*> RootActors;

    UWorld* World = GetWorld();
    if (!World)
    {
        return RootActors;
    }

    if (SceneName == GetCurrentSceneName())
    {
        // ��ȡ��ǰ�����ĸ�Actor
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (!It->GetAttachParentActor())
            {
                RootActors.Add(*It);
            }
        }
    }
    else
    {
        // ��ȡ���ͳ����ĸ�Actor
        ULevelStreaming* StreamingLevel = GetStreamingLevelByName(SceneName);
        if (StreamingLevel && StreamingLevel->GetLoadedLevel())
        {
            ULevel* Level = StreamingLevel->GetLoadedLevel();
            RootActors = Level->Actors;
        }
    }

    return RootActors;
}

TArray<AActor*> ULoadSceneManager::GetRootActorsInAllLoadedScenes() const
{
    TArray<AActor*> AllRootActors;

    TArray<FString> LoadedScenes = GetAllLoadedScenes();
    for (const FString& SceneName : LoadedScenes)
    {
        AllRootActors.Append(GetRootActorsInScene(SceneName));
    }

    return AllRootActors;
}

// ========== �첽������� ==========

ESceneLoadState ULoadSceneManager::GetAsyncRequestState(const FString& RequestId) const
{
    const FSceneAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    return Request ? Request->LoadState : ESceneLoadState::Failed;
}

float ULoadSceneManager::GetAsyncRequestProgress(const FString& RequestId) const
{
    const FSceneAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    return Request ? Request->Progress : 0.0f;
}

void ULoadSceneManager::CancelAsyncRequest(const FString& RequestId)
{
    StopProgressTimer(RequestId);
    AsyncRequests.Remove(RequestId);
    LoadCallbacks.Remove(RequestId);
    UnloadCallbacks.Remove(RequestId);
}

// ========== ���Թ��� ==========

void ULoadSceneManager::PrintAllScenesInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== All Scenes Info ==="));

    int32 SceneCount = GetSceneCount();
    UE_LOG(LogTemp, Log, TEXT("Total Scenes: %d"), SceneCount);

    for (int32 i = 0; i < SceneCount; i++)
    {
        FSceneInfo Info = GetSceneInfo(i);
        UE_LOG(LogTemp, Log, TEXT("  [%d] %s - %s"), i, *Info.SceneName, *UEnum::GetValueAsString(Info.LoadState));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Scenes Info ==="));
}

void ULoadSceneManager::PrintAllStreamingLevelsInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== Streaming Levels Info ==="));

    TArray<FStreamingLevelInfo> LevelsInfo = GetAllStreamingLevelsInfo();
    UE_LOG(LogTemp, Log, TEXT("Total Streaming Levels: %d"), LevelsInfo.Num());

    for (const FStreamingLevelInfo& LevelInfo : LevelsInfo)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s - Loaded: %s, Visible: %s"),
            *LevelInfo.LevelName,
            LevelInfo.bIsLoaded ? TEXT("Yes") : TEXT("No"),
            LevelInfo.bShouldBeVisible ? TEXT("Yes") : TEXT("No"));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Streaming Levels Info ==="));
}

void ULoadSceneManager::PrintAllAsyncRequests()
{
    UE_LOG(LogTemp, Log, TEXT("=== Async Requests ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Requests: %d"), AsyncRequests.Num());

    for (const auto& RequestPair : AsyncRequests)
    {
        const FSceneAsyncLoadRequest& Request = RequestPair.Value;
        UE_LOG(LogTemp, Log, TEXT("  %s: %s - %s (%.2f%%)"),
            *Request.RequestId,
            *Request.SceneName,
            *UEnum::GetValueAsString(Request.LoadState),
            Request.Progress * 100.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Async Requests ==="));
}

void ULoadSceneManager::PrintAllAvailableMaps()
{
    UE_LOG(LogTemp, Log, TEXT("=== Available Maps ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Maps: %d"), AvailableMaps.Num());

    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s -> %s (In Build: %s)"),
            *MapInfo.MapName,
            *MapInfo.DisplayName,
            MapInfo.bIsInBuildSettings ? TEXT("Yes") : TEXT("No"));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Available Maps ==="));
}

// ========== ��ͼ�ʲ����� ==========

TArray<TSoftObjectPtr<UWorld>> ULoadSceneManager::GetAllAvailableMapAssets() const
{
    TArray<TSoftObjectPtr<UWorld>> MapAssets;

    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        TSoftObjectPtr<UWorld> MapAsset = TSoftObjectPtr<UWorld>(FSoftObjectPath(MapInfo.MapPath));
        if (MapAsset.IsValid())
        {
            MapAssets.Add(MapAsset);
        }
    }

    return MapAssets;
}

TArray<TSoftObjectPtr<UWorld>> ULoadSceneManager::FindMapAssetsInFolder(const FString& FolderPath) const
{
    TArray<TSoftObjectPtr<UWorld>> FoundMapAssets;
    FString SanitizedPath = SanitizeMapPath(FolderPath);

    for (const FMapFileInfo& MapInfo : AvailableMaps)
    {
        if (MapInfo.MapPath.StartsWith(SanitizedPath))
        {
            TSoftObjectPtr<UWorld> MapAsset = TSoftObjectPtr<UWorld>(FSoftObjectPath(MapInfo.MapPath));
            if (MapAsset.IsValid())
            {
                FoundMapAssets.Add(MapAsset);
            }
        }
    }

    return FoundMapAssets;
}

bool ULoadSceneManager::IsMapAssetValid(const TSoftObjectPtr<UWorld>& MapAsset) const
{
    return MapAsset.IsValid() || !MapAsset.ToSoftObjectPath().IsNull();
}

FString ULoadSceneManager::GetMapAssetDisplayName(const TSoftObjectPtr<UWorld>& MapAsset) const
{
    if (IsMapAssetValid(MapAsset))
    {
        FString AssetPath = MapAsset.ToSoftObjectPath().GetAssetPathString();
        return GetMapDisplayName(AssetPath);
    }

    return FString();
}

// ========== �����Ļص�����ʵ�� ==========

void ULoadSceneManager::OnAsyncLoadComplete()
{
    // ��ȡ���ô˺�����LatentAction��UUID
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // ͨ����ǰִ�������Ļ�ȡUUID
    int32 CurrentUUID = 0;

    // ���������첽�����ҵ�ƥ���UUID
    for (auto& RequestPair : AsyncRequests)
    {
        FSceneAsyncLoadRequest& Request = RequestPair.Value;

        // �����������Ƿ�Ӧ�����
        if (Request.LoadState == ESceneLoadState::Loading)
        {
            // ��ʵ����Ŀ�У�����Ӧ�ü��ʵ�ʵļ���״̬
            // �򻯴���������سɹ�
            CompleteAsyncRequest(Request.RequestId, true);
            break;
        }
    }
}

void ULoadSceneManager::OnAsyncUnloadComplete()
{
    // ��ȡ���ô˺�����LatentAction��UUID
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // ͨ����ǰִ�������Ļ�ȡUUID
    int32 CurrentUUID = 0;

    // ���������첽�����ҵ�ƥ���UUID
    for (auto& RequestPair : AsyncRequests)
    {
        FSceneAsyncLoadRequest& Request = RequestPair.Value;

        // �����������Ƿ�Ӧ�����
        if (Request.LoadState == ESceneLoadState::Loading)
        {
            // ��ʵ����Ŀ�У�����Ӧ�ü��ʵ�ʵ�ж��״̬
            // �򻯴�������ж�سɹ�
            CompleteAsyncRequest(Request.RequestId, true);
            break;
        }
    }
}

void ULoadSceneManager::OnStreamingLevelLoaded()
{
    // ��ȡ�������¼������͹ؿ�
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // ���Ҹոռ�����ɵ����͹ؿ�
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (StreamingLevel && StreamingLevel->IsLevelLoaded())
        {
            FString LevelName = StreamingLevel->GetWorldAssetPackageName();
            UE_LOG(LogTemp, Log, TEXT("Streaming level loaded: %s"), *LevelName);

            // ���������Ӷ���Ĵ����߼�������֪ͨ����ϵͳ
            break;
        }
    }
}

void ULoadSceneManager::OnStreamingLevelUnloaded()
{
    // ��ȡ�������¼������͹ؿ�
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // ���Ҹո�ж�ص����͹ؿ�
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (StreamingLevel && !StreamingLevel->IsLevelLoaded())
        {
            FString LevelName = StreamingLevel->GetWorldAssetPackageName();
            UE_LOG(LogTemp, Log, TEXT("Streaming level unloaded: %s"), *LevelName);

            // ���������Ӷ���Ĵ����߼�������֪ͨ����ϵͳ
            break;
        }
    }
}

// ========== �ڲ�ʵ�� ==========

FString ULoadSceneManager::GenerateRequestId() const
{
    return FGuid::NewGuid().ToString();
}

bool ULoadSceneManager::IsValidSceneIndex(int32 SceneIndex) const
{
    return SceneIndex >= 0 && SceneIndex < GetSceneCount();
}

FString ULoadSceneManager::GetSceneNameFromIndex(int32 SceneIndex) const
{
    if (IsValidSceneIndex(SceneIndex) && AvailableMaps.IsValidIndex(SceneIndex))
    {
        return AvailableMaps[SceneIndex].MapName;
    }
    return FString();
}

int32 ULoadSceneManager::GetSceneIndexFromName(const FString& SceneName) const
{
    for (int32 i = 0; i < AvailableMaps.Num(); i++)
    {
        if (AvailableMaps[i].MapName == SceneName)
        {
            return i;
        }
    }
    return -1;
}

void ULoadSceneManager::UpdateAsyncRequestProgress(const FString& RequestId, float Progress)
{
    FSceneAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    if (Request)
    {
        Request->Progress = Progress;
        OnSceneLoadProgress.Broadcast(RequestId, Progress);
    }
}

void ULoadSceneManager::CompleteAsyncRequest(const FString& RequestId, bool bSuccess)
{
    FSceneAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    if (Request)
    {
        Request->LoadState = bSuccess ? ESceneLoadState::Loaded : ESceneLoadState::Failed;
        Request->Progress = 1.0f;

        StopProgressTimer(RequestId);

        if (bSuccess)
        {
            OnSceneLoadComplete.Broadcast(RequestId);

            // ִ�лص�
            FOnSceneLoadedCallback* Callback = LoadCallbacks.Find(RequestId);
            if (Callback && Callback->IsBound())
            {
                Callback->Execute(Request->SceneName);
            }

            UE_LOG(LogTemp, Log, TEXT("Async request completed successfully: %s -> %s"),
                *RequestId, *Request->SceneName);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Async request failed: %s -> %s"),
                *RequestId, *Request->SceneName);
        }

        LoadCallbacks.Remove(RequestId);
    }
}

FString ULoadSceneManager::FindRequestIdByUUID(int32 UUID) const
{
    for (const auto& RequestPair : AsyncRequests)
    {
        if (RequestPair.Value.LatentActionUUID == UUID)
        {
            return RequestPair.Key;
        }
    }
    return FString();
}

FString ULoadSceneManager::ExtractMapNameFromFullPath(const FString& FullPath) const
{
    FString CleanPath = FullPath;

    // �Ƴ� .umap ��׺
    CleanPath.RemoveFromEnd(TEXT(".umap"));

    // �������һ��б��
    int32 LastSlash = 0;
    if (CleanPath.FindLastChar('/', LastSlash))
    {
        return CleanPath.RightChop(LastSlash + 1);
    }

    return CleanPath;
}

FString ULoadSceneManager::SanitizeMapPath(const FString& MapPath) const
{
    FString SanitizedPath = MapPath;

    // �Ƴ������ǰ׺�ͺ�׺
    SanitizedPath.RemoveFromStart(TEXT("/Game/"));
    SanitizedPath.RemoveFromEnd(TEXT("."));

    // ȷ��·������Ϸ���ݸ�Ŀ¼��ʼ
    if (!SanitizedPath.StartsWith(TEXT("/Game/")))
    {
        SanitizedPath = TEXT("/Game/") + SanitizedPath;
    }

    return SanitizedPath;
}

FString ULoadSceneManager::ExtractMapNameFromPath(const FString& MapPath) const
{
    FString CleanPath = MapPath;

    // �Ƴ�·��ǰ׺
    if (CleanPath.StartsWith(TEXT("/Game/")))
    {
        CleanPath = CleanPath.RightChop(6); // �Ƴ� "/Game/"
    }

    // �������һ��б��
    int32 LastSlash = 0;
    if (CleanPath.FindLastChar('/', LastSlash))
    {
        return CleanPath.RightChop(LastSlash + 1);
    }

    return CleanPath;
}

bool ULoadSceneManager::IsMapPath(const FString& Path) const
{
    return Path.EndsWith(TEXT(".umap")) || Path.Contains(TEXT("/Maps/"));
}

ULevelStreaming* ULoadSceneManager::GetStreamingLevelByName(const FString& LevelName) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (StreamingLevel && StreamingLevel->GetWorldAssetPackageName().Contains(LevelName))
        {
            return StreamingLevel;
        }
    }

    return nullptr;
}

ULevelStreaming* ULoadSceneManager::GetStreamingLevelByPath(const FString& LevelPath) const
{
    FString LevelName = ExtractMapNameFromPath(LevelPath);
    return GetStreamingLevelByName(LevelName);
}

void ULoadSceneManager::ScanForMapFiles()
{
    AvailableMaps.Empty();

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // �������е�ͼ�ʲ�
    TArray<FAssetData> MapAssets;
    AssetRegistry.GetAssetsByClass(FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("World")), MapAssets);

    for (const FAssetData& AssetData : MapAssets)
    {
        FMapFileInfo MapInfo;
        MapInfo.MapPath = AssetData.GetObjectPathString();
        MapInfo.MapName = ExtractMapNameFromPath(MapInfo.MapPath);
        MapInfo.DisplayName = AssetData.AssetName.ToString();
        MapInfo.bIsInBuildSettings = true; // �򻯴���

        AvailableMaps.Add(MapInfo);

        UE_LOG(LogTemp, Verbose, TEXT("Found map asset: %s -> %s"), *MapInfo.MapName, *MapInfo.DisplayName);
    }

    UE_LOG(LogTemp, Log, TEXT("Scanned %d available maps"), AvailableMaps.Num());
}

void ULoadSceneManager::StartProgressTimer(const FString& RequestId)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    FTimerHandle TimerHandle;

    // ʹ�� Lambda ���ʽ������ RequestId
    FTimerDelegate TimerDelegate;
    TimerDelegate.BindLambda([this, RequestId]()
        {
            this->UpdateProgress(RequestId);
        });

    World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.1f, true);
    ProgressTimerHandles.Add(RequestId, TimerHandle);
}

void ULoadSceneManager::StopProgressTimer(const FString& RequestId)
{
    FTimerHandle* TimerHandle = ProgressTimerHandles.Find(RequestId);
    if (TimerHandle)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(*TimerHandle);
        }
        ProgressTimerHandles.Remove(RequestId);
    }
}

void ULoadSceneManager::UpdateProgress(const FString& RequestId)
{
    // ģ����ȸ��£�ʵ����Ŀ��Ӧ�ø�����ʵ�ļ��ؽ���������
    FSceneAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    if (Request && Request->LoadState == ESceneLoadState::Loading)
    {
        float NewProgress = FMath::Min(Request->Progress + 0.1f, 0.9f);
        UpdateAsyncRequestProgress(RequestId, NewProgress);

        UE_LOG(LogTemp, Verbose, TEXT("Progress update for request %s: %.2f"), *RequestId, NewProgress);
    }
    else
    {
        // ������󲻴��ڻ��ڼ���״̬��ֹͣ��ʱ��
        StopProgressTimer(RequestId);
    }
}

UWorld* ULoadSceneManager::GetWorld() const
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