// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "LoadSceneManager.generated.h"

// 场景加载模式
UENUM(BlueprintType)
enum class ESceneLoadMode : uint8
{
    Single UMETA(DisplayName = "Single"),
    Additive UMETA(DisplayName = "Additive")
};

// 场景加载状态
UENUM(BlueprintType)
enum class ESceneLoadState : uint8
{
    NotLoaded UMETA(DisplayName = "Not Loaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Failed UMETA(DisplayName = "Failed")
};

// 场景信息
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

// 异步加载请求 - 重命名为避免冲突
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

    // 新增字段用于关联LatentActionInfo
    int32 LatentActionUUID;

    FSceneAsyncLoadRequest()
        : LoadState(ESceneLoadState::NotLoaded)
        , Progress(0.0f)
        , LatentActionUUID(0)
    {
    }
};

// 流送关卡信息
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

// 地图文件信息
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

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSceneLoadProgress, const FString&, RequestId, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneLoadComplete, const FString&, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSceneUnloadComplete, const FString&, RequestId);

// 简化的回调委托
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSceneLoadedCallback, const FString&, SceneName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSceneUnloadedCallback, const FString&, SceneName);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API ULoadSceneManager : public USingletonBase
{
    GENERATED_BODY()

    // 单例声明
    DECLARE_SINGLETON(ULoadSceneManager)

public:
    // 初始化场景管理器
    UFUNCTION(BlueprintCallable, Category = "Scene")
    void InitializeSceneManager();

    // 重写单例初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // 获取管理器实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Scene", meta = (DisplayName = "Get Scene Manager"))
    static ULoadSceneManager* GetSceneManager() { return GetInstance(); }

    // 构造函数
    ULoadSceneManager();
    virtual ~ULoadSceneManager() override;

    // ========== 同步场景加载 ==========

    // 通过索引同步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByIndex(int32 SceneIndex, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // 通过名称同步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByName(const FString& SceneName, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // 通过文件路径同步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByPath(const FString& MapPath, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // 通过 TSoftObjectPtr 同步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // 通过完整资产路径同步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadSceneByFullPath(const FString& FullMapPath, ESceneLoadMode Mode = ESceneLoadMode::Single);

    // 重新加载当前活动场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void ReloadCurrentScene();

    // 加载下一个场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadNextScene(bool bCyclical = false);

    // 加载上一个场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Sync")
    void LoadPreviousScene(bool bCyclical = false);

    // ========== 异步场景加载 ==========

    // 通过索引异步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByIndex(int32 SceneIndex, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // 通过名称异步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByName(const FString& SceneName, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // 通过文件路径异步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByPath(const FString& MapPath, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // 通过 TSoftObjectPtr 异步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByAsset(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // 通过完整资产路径异步加载场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadSceneAsyncByFullPath(const FString& FullMapPath, ESceneLoadMode Mode = ESceneLoadMode::Single, bool bActivateAfterLoad = true);

    // 异步重新加载当前场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString ReloadCurrentSceneAsync(bool bActivateAfterLoad = true);

    // 异步加载下一个场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadNextSceneAsync(bool bCyclical = false, bool bActivateAfterLoad = true);

    // 异步加载上一个场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    FString LoadPreviousSceneAsync(bool bCyclical = false, bool bActivateAfterLoad = true);

    // ========== 异步场景加载（带回调） ==========

// 异步加载场景带完成回调
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncWithCallback(int32 SceneIndex, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // 异步加载场景带路径和完成回调
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncByPathWithCallback(const FString& MapPath, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // 通过 TSoftObjectPtr 异步加载场景带回调
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void LoadSceneAsyncByAssetWithCallback(const TSoftObjectPtr<UWorld>& MapAsset, ESceneLoadMode Mode, bool bActivateAfterLoad, const FOnSceneLoadedCallback& OnComplete);

    // ========== 场景卸载 ==========

    // 卸载场景（通过名称）
    UFUNCTION(BlueprintCallable, Category = "Scene|Unload")
    FString UnloadSceneAsync(const FString& SceneName);

    // 卸载所有附加场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Unload")
    void UnloadAllAdditiveScenes();

    // ========== 地图流送管理 ==========

    // 加载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevel(const FString& LevelName, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // 通过路径加载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevelByPath(const FString& LevelPath, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // 通过 TSoftObjectPtr 加载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevelByAsset(const TSoftObjectPtr<UWorld>& MapAsset, bool bMakeVisibleAfterLoad = true, bool bShouldBlockOnLoad = false);

    // 卸载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void UnloadStreamingLevel(const FString& LevelName);

    // 设置流送关卡可见性
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void SetStreamingLevelVisible(const FString& LevelName, bool bVisible);

    // 获取所有流送关卡信息
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    TArray<FStreamingLevelInfo> GetAllStreamingLevelsInfo() const;

    // 检查流送关卡是否已加载
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    bool IsStreamingLevelLoaded(const FString& LevelName) const;

    // 批量加载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void LoadStreamingLevels(const TArray<FString>& LevelNames, bool bMakeVisibleAfterLoad = true);

    // 批量卸载流送关卡
    UFUNCTION(BlueprintCallable, Category = "Scene|Streaming")
    void UnloadStreamingLevels(const TArray<FString>& LevelNames);

    // ========== 地图文件管理 ==========

    // 获取所有可用的地图文件
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<FMapFileInfo> GetAllAvailableMaps() const;

    // 在指定文件夹中查找地图文件
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<FMapFileInfo> FindMapsInFolder(const FString& FolderPath) const;

    // 检查地图文件是否存在
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    bool DoesMapFileExist(const FString& MapPath) const;

    // 获取地图显示名称
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    FString GetMapDisplayName(const FString& MapPath) const;

    // ========== 场景信息查询 ==========

    // 获取当前活动场景名称
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FString GetCurrentSceneName() const;

    // 获取当前场景路径
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FString GetCurrentScenePath() const;

    // 获取当前场景索引
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    int32 GetCurrentSceneIndex() const;

    // 获取场景总数
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    int32 GetSceneCount() const;

    // 获取场景信息
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    FSceneInfo GetSceneInfo(int32 SceneIndex) const;

    // 检查场景索引是否有效
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    bool IsSceneIndexValid(int32 SceneIndex) const;

    // 检查场景名称是否存在
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    bool DoesSceneExist(const FString& SceneName) const;

    // 获取所有已加载的场景
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<FString> GetAllLoadedScenes() const;

    // 获取场景中的根Actor
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<AActor*> GetRootActorsInScene(const FString& SceneName) const;

    // 获取所有已加载场景的根Actor
    UFUNCTION(BlueprintCallable, Category = "Scene|Query")
    TArray<AActor*> GetRootActorsInAllLoadedScenes() const;

    // ========== 异步请求管理 ==========

    // 获取异步请求状态
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    ESceneLoadState GetAsyncRequestState(const FString& RequestId) const;

    // 获取异步请求进度
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    float GetAsyncRequestProgress(const FString& RequestId) const;

    // 取消异步请求
    UFUNCTION(BlueprintCallable, Category = "Scene|Async")
    void CancelAsyncRequest(const FString& RequestId);

    // ========== 调试工具 ==========

    // 打印所有场景信息
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllScenesInfo();

    // 打印所有流送关卡信息
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllStreamingLevelsInfo();

    // 打印所有异步请求信息
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllAsyncRequests();

    // 打印所有可用地图
    UFUNCTION(BlueprintCallable, Category = "Scene|Debug")
    void PrintAllAvailableMaps();

    // ========== 地图资产管理 ==========

// 获取所有可用的地图资产
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<TSoftObjectPtr<UWorld>> GetAllAvailableMapAssets() const;

    // 在指定文件夹中查找地图资产
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    TArray<TSoftObjectPtr<UWorld>> FindMapAssetsInFolder(const FString& FolderPath) const;

    // 检查地图资产是否有效
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    bool IsMapAssetValid(const TSoftObjectPtr<UWorld>& MapAsset) const;

    // 获取地图资产的显示名称
    UFUNCTION(BlueprintCallable, Category = "Scene|Maps")
    FString GetMapAssetDisplayName(const TSoftObjectPtr<UWorld>& MapAsset) const;

    // ========== 委托 ==========

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneLoadProgress OnSceneLoadProgress;

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneLoadComplete OnSceneLoadComplete;

    UPROPERTY(BlueprintAssignable, Category = "Scene|Events")
    FOnSceneUnloadComplete OnSceneUnloadComplete;

private:
    // 异步加载请求映射 - 使用重命名的结构体
    TMap<FString, FSceneAsyncLoadRequest> AsyncRequests;

    // 场景信息缓存
    TMap<FString, FSceneInfo> SceneInfoCache;

    // 回调映射
    TMap<FString, FOnSceneLoadedCallback> LoadCallbacks;
    TMap<FString, FOnSceneUnloadedCallback> UnloadCallbacks;

    // 内部方法
    FString GenerateRequestId() const;
    bool IsValidSceneIndex(int32 SceneIndex) const;
    FString GetSceneNameFromIndex(int32 SceneIndex) const;
    int32 GetSceneIndexFromName(const FString& SceneName) const;
    void UpdateAsyncRequestProgress(const FString& RequestId, float Progress);
    void CompleteAsyncRequest(const FString& RequestId, bool bSuccess);

    // 路径处理
    FString SanitizeMapPath(const FString& MapPath) const;
    FString ExtractMapNameFromPath(const FString& MapPath) const;
    bool IsMapPath(const FString& Path) const;

    // 流送关卡管理
    ULevelStreaming* GetStreamingLevelByName(const FString& LevelName) const;
    ULevelStreaming* GetStreamingLevelByPath(const FString& LevelPath) const;

    // 完整的回调函数声明
    UFUNCTION()
    void OnAsyncLoadComplete();

    UFUNCTION()
    void OnAsyncUnloadComplete();

    UFUNCTION()
    void OnStreamingLevelLoaded();

    UFUNCTION()
    void OnStreamingLevelUnloaded();

    // 地图文件扫描
    void ScanForMapFiles();
    TArray<FMapFileInfo> AvailableMaps;

    // 计时器用于进度更新
    void StartProgressTimer(const FString& RequestId);
    void StopProgressTimer(const FString& RequestId);
    void UpdateProgress(const FString& RequestId);

    // 辅助方法：通过UUID查找请求ID
    FString FindRequestIdByUUID(int32 UUID) const;

    FString ExtractMapNameFromFullPath(const FString& FullPath) const;

    // 计时器句柄
    TMap<FString, FTimerHandle> ProgressTimerHandles;

    // 获取World的辅助方法
    UWorld* GetWorld() const override;
};