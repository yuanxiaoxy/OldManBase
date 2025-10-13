// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "LoadSceneManager.generated.h"

// ��������ģʽ
UENUM(BlueprintType)
enum class ESceneLoadMode : uint8
{
    Single UMETA(DisplayName = "Single"),
    Additive UMETA(DisplayName = "Additive")
};

// ��������״̬
UENUM(BlueprintType)
enum class ESceneLoadState : uint8
{
    NotLoaded UMETA(DisplayName = "Not Loaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Failed UMETA(DisplayName = "Failed")
};

// ������Ϣ
USTRUCT(BlueprintType)
struct FSceneInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString SceneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString ScenePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    ESceneLoadState LoadState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    int32 BuildIndex;

    FSceneInfo()
        : LoadState(ESceneLoadState::NotLoaded)
        , BuildIndex(-1)
    {
    }
};

// �첽�������� - ������Ϊ�����ͻ
USTRUCT(BlueprintType)
struct FSceneAsyncLoadRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString SceneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    ESceneLoadState LoadState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    float Progress;

    // �����ֶ����ڹ���LatentActionInfo
    int32 LatentActionUUID;

    FSceneAsyncLoadRequest()
        : LoadState(ESceneLoadState::NotLoaded)
        , Progress(0.0f)
        , LatentActionUUID(0)
    {
    }
};

// ���͹ؿ���Ϣ
USTRUCT(BlueprintType)
struct FStreamingLevelInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString PackageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    bool bShouldBeLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    bool bShouldBeVisible;

    FStreamingLevelInfo()
        : bIsLoaded(false)
        , bShouldBeLoaded(false)
        , bShouldBeVisible(false)
    {
    }
};

// ��ͼ�ļ���Ϣ
USTRUCT(BlueprintType)
struct FMapFileInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString MapName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString MapPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    bool bIsInBuildSettings;

    FMapFileInfo()
        : bIsInBuildSettings(false)
    {
    }
};

// ί������
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSceneLoadProgress, const FString&, RequestId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneLoadComplete, const FString&, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneUnloadComplete, const FString&, RequestId);

// �򻯵Ļص�ί��
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSceneLoadedCallback, const FString&, SceneName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSceneUnloadedCallback, const FString&, SceneName);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API ULoadSceneManager : public USingletonBase
{
    GENERATED_BODY()

    // ��������
    DECLARE_SINGLETON(ULoadSceneManager)

public:
    // ��ʼ������������
    UFUNCTION(BlueprintCallable, Category = "Scene")
    void InitializeSceneManager();

    // ��д������ʼ������
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // ��ȡ������ʵ������ͼ�ɵ��÷���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Scene", meta = (DisplayName = "Get Scene Manager"))
    static ULoadSceneManager* GetSceneManager() { return GetInstance(); }

    // ���캯��
    ULoadSceneManager();
    virtual ~ULoadSceneManager() override;

    // ========== ͬ���������� ==========

    // ͨ������ͬ�����س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByIndex(int32 SceneIndex, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // ͨ������ͬ�����س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByName(const FString& SceneName, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // ͨ���ļ�·��ͬ�����س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByPath(const FString& MapPath, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // ͨ�� TSoftObjectPtr ͬ�����س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // ͨ�������ʲ�·��ͬ�����س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByFullPath(const FString& FullMapPath, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // ���¼��ص�ǰ�����
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void ReloadCurrentScene();

    // ������һ������
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadNextScene(bool bCyclical = false);

    // ������һ������
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadPreviousScene(bool bCyclical = false);

    // ========== �첽�������� ==========

    // ͨ�������첽���س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByIndex(int32 SceneIndex, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // ͨ�������첽���س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByName(const FString& SceneName, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // ͨ���ļ�·���첽���س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByPath(const FString& MapPath, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // ͨ�� TSoftObjectPtr �첽���س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // ͨ�������ʲ�·���첽���س���
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByFullPath(const FString& FullMapPath, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // �첽���¼��ص�ǰ����
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString ReloadCurrentSceneAsync(bool bActivateAfterLoad = true);

    // �첽������һ������
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadNextSceneAsync(bool bCyclical = false, bool bActivateAfterLoad = true);

    // �첽������һ������
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadPreviousSceneAsync(bool bCyclical = false, bool bActivateAfterLoad = true);

    // ========== �첽�������أ����ص��� ==========

// �첽���س�������ɻص�
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncWithCallback(int32 SceneIndex, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // �첽���س�����·������ɻص�
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncByPathWithCallback(const FString& MapPath, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // ͨ�� TSoftObjectPtr �첽���س������ص�
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncByAssetWithCallback(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // ========== ����ж�� ==========

    // ж�س�����ͨ�����ƣ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Unload")
    FString UnloadSceneAsync(const FString& SceneName);

    // ж�����и��ӳ���
    UFUNCTION(BlueprintCallable, Category = "Scene|Unload")
    void UnloadAllAdditiveScenes();

    // ========== ��ͼ���͹��� ==========

    // �������͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevel(const FString& LevelName, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // ͨ��·���������͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevelByPath(const FString& LevelPath, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // ͨ�� TSoftObjectPtr �������͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevelByAsset(const TSoftObjectPtr<UWorld>& MapAsset, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // ж�����͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void UnloadStreamingLevel(const FString& LevelName);

    // �������͹ؿ��ɼ���
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void SetStreamingLevelVisible(const FString& LevelName, bool bVisible);

    // ��ȡ�������͹ؿ���Ϣ
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    TArray<FStreamingLevelInfo> GetAllStreamingLevelsInfo() const;

    // ������͹ؿ��Ƿ��Ѽ���
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    bool IsStreamingLevelLoaded(const FString& LevelName) const;

    // �����������͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevels(const TArray<FString>& LevelNames, bool bMakeVisibleAfterLoad = true);

    // ����ж�����͹ؿ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void UnloadStreamingLevels(const TArray<FString>& LevelNames);

    // ========== ��ͼ�ļ����� ==========

    // ��ȡ���п��õĵ�ͼ�ļ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<FMapFileInfo> GetAllAvailableMaps() const;

    // ��ָ���ļ����в��ҵ�ͼ�ļ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<FMapFileInfo> FindMapsInFolder(const FString& FolderPath) const;

    // ����ͼ�ļ��Ƿ����
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    bool DoesMapFileExist(const FString& MapPath) const;

    // ��ȡ��ͼ��ʾ����
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    FString GetMapDisplayName(const FString& MapPath) const;

    // ========== ������Ϣ��ѯ ==========

    // ��ȡ��ǰ���������
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FString GetCurrentSceneName() const;

    // ��ȡ��ǰ����·��
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FString GetCurrentScenePath() const;

    // ��ȡ��ǰ��������
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    int32 GetCurrentSceneIndex() const;

    // ��ȡ��������
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    int32 GetSceneCount() const;

    // ��ȡ������Ϣ
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FSceneInfo GetSceneInfo(int32 SceneIndex) const;

    // ��鳡�������Ƿ���Ч
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    bool IsSceneIndexValid(int32 SceneIndex) const;

    // ��鳡�������Ƿ����
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    bool DoesSceneExist(const FString& SceneName) const;

    // ��ȡ�����Ѽ��صĳ���
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<FString> GetAllLoadedScenes() const;

    // ��ȡ�����еĸ�Actor
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<AActor*> GetRootActorsInScene(const FString& SceneName) const;

    // ��ȡ�����Ѽ��س����ĸ�Actor
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<AActor*> GetRootActorsInAllLoadedScenes() const;

    // ========== �첽������� ==========

    // ��ȡ�첽����״̬
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    ESceneLoadState GetAsyncRequestState(const FString& RequestId) const;

    // ��ȡ�첽�������
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    float GetAsyncRequestProgress(const FString& RequestId) const;

    // ȡ���첽����
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void CancelAsyncRequest(const FString& RequestId);

    // ========== ���Թ��� ==========

    // ��ӡ���г�����Ϣ
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllScenesInfo();

    // ��ӡ�������͹ؿ���Ϣ
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllStreamingLevelsInfo();

    // ��ӡ�����첽������Ϣ
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllAsyncRequests();

    // ��ӡ���п��õ�ͼ
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllAvailableMaps();

    // ========== ��ͼ�ʲ����� ==========

// ��ȡ���п��õĵ�ͼ�ʲ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<TSoftObjectPtr<UWorld>> GetAllAvailableMapAssets() const;

    // ��ָ���ļ����в��ҵ�ͼ�ʲ�
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<TSoftObjectPtr<UWorld>> FindMapAssetsInFolder(const FString& FolderPath) const;

    // ����ͼ�ʲ��Ƿ���Ч
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    bool IsMapAssetValid(const TSoftObjectPtr<UWorld>& MapAsset) const;

    // ��ȡ��ͼ�ʲ�����ʾ����
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    FString GetMapAssetDisplayName(const TSoftObjectPtr<UWorld>& MapAsset) const;

    // ========== ί�� ==========

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneLoadProgress OnSceneLoadProgress;

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneLoadComplete OnSceneLoadComplete;

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneUnloadComplete OnSceneUnloadComplete;

private:
    // �첽��������ӳ�� - ʹ���������Ľṹ��
    TMap<FString, FSceneAsyncLoadRequest> AsyncRequests;

    // ������Ϣ����
    TMap<FString, FSceneInfo> SceneInfoCache;

    // �ص�ӳ��
    TMap<FString, FOnSceneLoadedCallback> LoadCallbacks;
    TMap<FString, FOnSceneUnloadedCallback> UnloadCallbacks;

    // �ڲ�����
    FString GenerateRequestId() const;
    bool IsValidSceneIndex(int32 SceneIndex) const;
    FString GetSceneNameFromIndex(int32 SceneIndex) const;
    int32 GetSceneIndexFromName(const FString& SceneName) const;
    void UpdateAsyncRequestProgress(const FString& RequestId, float Progress);
    void CompleteAsyncRequest(const FString& RequestId, bool bSuccess);

    // ·������
    FString SanitizeMapPath(const FString& MapPath) const;
    FString ExtractMapNameFromPath(const FString& MapPath) const;
    bool IsMapPath(const FString& Path) const;

    // ���͹ؿ�����
    ULevelStreaming* GetStreamingLevelByName(const FString& LevelName) const;
    ULevelStreaming* GetStreamingLevelByPath(const FString& LevelPath) const;

    // �����Ļص���������
    UFUNCTION()
    void OnAsyncLoadComplete();

    UFUNCTION()
    void OnAsyncUnloadComplete();

    UFUNCTION()
    void OnStreamingLevelLoaded();

    UFUNCTION()
    void OnStreamingLevelUnloaded();

    // ��ͼ�ļ�ɨ��
    void ScanForMapFiles();
    TArray<FMapFileInfo> AvailableMaps;

    // ��ʱ�����ڽ��ȸ���
    void StartProgressTimer(const FString& RequestId);
    void StopProgressTimer(const FString& RequestId);
    void UpdateProgress(const FString& RequestId);

    // ����������ͨ��UUID��������ID
    FString FindRequestIdByUUID(int32 UUID) const;

    FString ExtractMapNameFromFullPath(const FString& FullPath) const;

    // ��ʱ�����
    TMap<FString, FTimerHandle> ProgressTimerHandles;

    // ��ȡWorld�ĸ�������
    UWorld* GetWorld() const override;
};