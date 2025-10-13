// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "ResourceTableRow.h"
#include "ResourceManager.generated.h"

// 资源加载状态
UENUM(BlueprintType)
enum class EResourceLoadState : uint8
{
    NotLoaded UMETA(DisplayName = "Not Loaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Failed UMETA(DisplayName = "Failed")
};

// 资源信息结构
USTRUCT(BlueprintType)
struct FResourceInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourcePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    TSubclassOf<UObject> ResourceClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceLoadState LoadState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    UObject* LoadedResource;

    FResourceInfo()
        : LoadState(EResourceLoadState::NotLoaded)
        , LoadedResource(nullptr)
    {
    }
};

// 异步加载请求ID
USTRUCT(BlueprintType)
struct FAsyncLoadRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString RequestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FString ResourcePath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceLoadState LoadState;

    FAsyncLoadRequest()
        : LoadState(EResourceLoadState::NotLoaded)
    {
    }

    FAsyncLoadRequest(const FString& InRequestId, const FString& InResourcePath)
        : RequestId(InRequestId)
        , ResourcePath(InResourcePath)
        , LoadState(EResourceLoadState::NotLoaded)
    {
    }
};

// 资源加载完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceFinishLoadedSignature, const FString&, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourcesFinishLoadedSignature, const FString&, RequestId);

// 新的简化回调委托 - 直接传递加载的资源
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnResourceLoadedCallback, UObject*, LoadedResource);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnResourcesLoadedCallback, const TArray<UObject*>&, LoadedResources);

// 静态委托用于C++成员函数指针
DECLARE_DELEGATE_OneParam(FOnResourceLoadedStaticDelegate, UObject*);
DECLARE_DELEGATE_OneParam(FOnResourcesLoadedStaticDelegate, const TArray<UObject*>&);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UResourceManager : public USingletonBase
{
    GENERATED_BODY()

    // 声明单例
    DECLARE_SINGLETON(UResourceManager)

public:
    // 初始化资源管理器
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void InitializeResourceManager();

    // 重写初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); };

    // 获取实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource", meta = (DisplayName = "Get Resource Manager"))
    static UResourceManager* GetResourceManager() { return GetInstance(); }

    // 设置资源数据表
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void SetResourceDataTable(UDataTable* InResourceDataTable);


    // 默认构造函数
    UResourceManager();

    // 析构函数
    virtual ~UResourceManager() override;

    // ========== 蓝图可分配委托 ==========

    // 单个资源加载完成委托
    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourceFinishLoadedSignature OnResourceFinishLoaded;

    // 多个资源加载完成委托
    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourcesFinishLoadedSignature OnResourcesFinishLoaded;

    // ========== 基础资源操作 ==========

    // 同步加载单个资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* LoadResourceSync(const FString& ResourcePath);

    // 通过资源ID加载资源（同步）
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    UObject* LoadResourceByID(const FName& ResourceID);

    // 同步加载指定类型的单个资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* LoadResourceSyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass);

    // 同步加载文件夹下所有资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> LoadResourcesInFolderSync(const FString& FolderPath);

    // 同步加载文件夹下指定类型的所有资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> LoadResourcesInFolderSyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // ========== 异步资源操作（使用请求ID） ==========

    // 异步加载单个资源 - 返回请求ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourceAsync(const FString& ResourcePath);

    // 异步加载指定类型的单个资源 - 返回请求ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourceAsyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass);

    // 异步加载文件夹下所有资源 - 返回请求ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourcesInFolderAsync(const FString& FolderPath);

    // 异步加载文件夹下指定类型的所有资源 - 返回请求ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourcesInFolderAsyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // ========== 简化的异步资源操作（直接绑定回调） ==========

    // 异步加载单个资源 - 直接绑定回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resource Async With Callback"))
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, const FOnResourceLoadedCallback& Callback);

    // 异步加载指定类型的单个资源 - 直接绑定回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resource Async By Class With Callback"))
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedCallback& Callback);

    // 通过资源ID加载资源（异步）- 返回请求ID
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    FString LoadResourceByIDAsync(const FName& ResourceID);

    // 通过资源ID加载资源（异步）- 直接回调
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void LoadResourceByIDWithCallback(const FName& ResourceID, const FOnResourceLoadedCallback& Callback);

    // 按分类加载多个资源
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void LoadResourcesByCategory(EResourceCategory Category, const FOnResourcesLoadedCallback& Callback);

    // 异步加载文件夹下所有资源 - 直接绑定回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resources In Folder Async With Callback"))
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, const FOnResourcesLoadedCallback& Callback);

    // 异步加载文件夹下指定类型的所有资源 - 直接绑定回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resources In Folder Async By Class With Callback"))
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedCallback& Callback);

    // ========== C++模板方法 - 直接传递成员函数指针 ==========

    // 异步加载单个资源 - 直接绑定成员函数指针
    template<typename T>
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, T* Object, void(T::* Function)(UObject*))
    {
        FOnResourceLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourceAsyncWithStaticCallback(ResourcePath, StaticDelegate);
    }

    // 异步加载单个资源 - 无参成员函数指针
    template<typename T>
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, T* Object, void(T::* Function)())
    {
        // 使用lambda包装无参函数，忽略加载的资源参数
        FOnResourceLoadedStaticDelegate StaticDelegate = FOnResourceLoadedStaticDelegate::CreateLambda([Object, Function](UObject* LoadedResource) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourceAsyncWithStaticCallback(ResourcePath, StaticDelegate);
    }

    // 异步加载指定类型的单个资源 - 直接绑定成员函数指针（带参数）
    template<typename T>
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)(UObject*))
    {
        FOnResourceLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourceAsyncByClassWithStaticCallback(ResourcePath, ResourceClass, StaticDelegate);
    }

    // 异步加载指定类型的单个资源 - 无参成员函数指针
    template<typename T>
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)())
    {
        // 使用lambda包装无参函数，忽略加载的资源参数
        FOnResourceLoadedStaticDelegate StaticDelegate = FOnResourceLoadedStaticDelegate::CreateLambda([Object, Function](UObject* LoadedResource) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourceAsyncByClassWithStaticCallback(ResourcePath, ResourceClass, StaticDelegate);
    }

    // 通过资源ID加载资源（异步）- 成员函数指针回调
    template<typename T>
    void LoadResourceByIDWithCallback(const FName& ResourceID, T* Object, void(T::* Function)(UObject*))
    {
        FString ResourcePath = GetResourcePathByID(ResourceID);
        if (!ResourcePath.IsEmpty())
        {
            LoadResourceAsyncWithCallback(ResourcePath, Object, Function);
        }
    }

    template<typename T>
    void LoadResourceByIDWithCallback(const FName& ResourceID, T* Object, void(T::* Function)())
    {
        FString ResourcePath = GetResourcePathByID(ResourceID);
        if (!ResourcePath.IsEmpty())
        {
            LoadResourceAsyncWithCallback(ResourcePath, Object, Function);
        }
    }

    // 异步加载文件夹下所有资源 - 直接绑定成员函数指针（带参数）
    template<typename T>
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, T* Object, void(T::* Function)(const TArray<UObject*>&))
    {
        FOnResourcesLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourcesInFolderAsyncWithStaticCallback(FolderPath, StaticDelegate);
    }

    // 异步加载文件夹下所有资源 - 无参成员函数指针
    template<typename T>
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, T* Object, void(T::* Function)())
    {
        // 使用lambda包装无参函数，忽略加载的资源数组参数
        FOnResourcesLoadedStaticDelegate StaticDelegate = FOnResourcesLoadedStaticDelegate::CreateLambda([Object, Function](const TArray<UObject*>& LoadedResources) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourcesInFolderAsyncWithStaticCallback(FolderPath, StaticDelegate);
    }

    // 异步加载文件夹下指定类型的所有资源 - 直接绑定成员函数指针（带参数）
    template<typename T>
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)(const TArray<UObject*>&))
    {
        FOnResourcesLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(FolderPath, ResourceClass, StaticDelegate);
    }

    // 异步加载文件夹下指定类型的所有资源 - 无参成员函数指针
    template<typename T>
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)())
    {
        // 使用lambda包装无参函数，忽略加载的资源数组参数
        FOnResourcesLoadedStaticDelegate StaticDelegate = FOnResourcesLoadedStaticDelegate::CreateLambda([Object, Function](const TArray<UObject*>& LoadedResources) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(FolderPath, ResourceClass, StaticDelegate);
    }

    // ========== 异步请求管理 ==========

    // 获取异步请求状态
    UFUNCTION(BlueprintCallable, Category = "Resource")
    EResourceLoadState GetAsyncRequestState(const FString& RequestId) const;

    // 获取异步请求加载的资源（单个）
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* GetAsyncRequestResource(const FString& RequestId) const;

    // 获取异步请求加载的资源（多个）
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> GetAsyncRequestResources(const FString& RequestId) const;

    // 取消异步请求
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void CancelAsyncRequest(const FString& RequestId);

    // ========== 资源缓存管理 ==========

    // 预加载所有标记为预加载的资源
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void PreloadMarkedResources();

    // 将资源添加到缓存
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void AddToCache(const FString& ResourcePath, UObject* Resource);

    // 从缓存获取资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* GetFromCache(const FString& ResourcePath);

    // 检查资源是否在缓存中
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool IsInCache(const FString& ResourcePath);

    // 从缓存移除资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RemoveFromCache(const FString& ResourcePath);

    // 清空资源缓存
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void ClearCache();

    // 获取缓存大小
    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetCacheSize() const;

    // ========== 资源信息查询 ==========

    // 获取资源信息
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FResourceInfo GetResourceInfo(const FString& ResourcePath);

    // 获取资源信息
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    bool GetResourceInfoByID(const FName& ResourceID, FResourceTableRow& OutInfo) const;

    // 检查资源是否存在
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool DoesResourceExist(const FString& ResourcePath);

    // 检查资源ID是否存在
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    bool DoesResourceIDExist(const FName& ResourceID) const;

    // 获取文件夹下所有资源路径
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<FString> GetResourcePathsInFolder(const FString& FolderPath);

    // 获取文件夹下指定类型的所有资源路径
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<FString> GetResourcePathsInFolderByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // 获取分类下的所有资源ID
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    TArray<FName> GetResourceIDsByCategory(EResourceCategory Category) const;

    // 获取资源路径
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    FString GetResourcePathByID(const FName& ResourceID) const;

    // ========== 资源卸载管理 ==========

    // 卸载单个资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void UnloadResource(const FString& ResourcePath);

    // 卸载文件夹下所有资源
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void UnloadResourcesInFolder(const FString& FolderPath);

    // 强制垃圾回收
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void ForceGarbageCollection();

    // ========== 调试工具 ==========

    // 打印缓存信息
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintCacheInfo();

    // 打印文件夹资源列表
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintFolderResources(const FString& FolderPath);

    // 打印所有异步请求状态
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintAsyncRequests();

private:
    // 流式加载管理器
    FStreamableManager StreamableManager;

    // 资源缓存
    TMap<FString, UObject*> ResourceCache;

    // 异步加载句柄
    TMap<FString, TSharedPtr<FStreamableHandle>> AsyncHandles;

    // 异步请求信息
    TMap<FString, FAsyncLoadRequest> AsyncRequests;

    // 单个资源请求结果
    TMap<FString, UObject*> SingleResourceResults;

    // 多个资源请求结果
    TMap<FString, TArray<UObject*>> MultiResourceResults;

    // 简化回调映射
    TMap<FString, FOnResourceLoadedCallback> SingleResourceCallbacks;
    TMap<FString, FOnResourcesLoadedCallback> MultiResourceCallbacks;

    // 静态回调映射
    TMap<FString, FOnResourceLoadedStaticDelegate> SingleResourceStaticCallbacks;
    TMap<FString, FOnResourcesLoadedStaticDelegate> MultiResourceStaticCallbacks;

    // 内部加载方法
    UObject* InternalLoadResourceSync(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass = nullptr);
    TArray<UObject*> InternalLoadResourcesInFolderSync(const FString& FolderPath, TSubclassOf<UObject> ResourceClass = nullptr);

    // 资源数据表
    UPROPERTY()
    UDataTable* ResourceDataTable;

    // 资源ID到路径的快速查找表
    TMap<FName, FString> ResourceIDToPathMap;

    // 分类到资源ID的映射
    TMap<EResourceCategory, TArray<FName>> CategoryToResourceIDsMap;

    // 内部异步加载回调
    void HandleSingleResourceLoaded(FString RequestId);
    void HandleFolderResourcesLoaded(FString RequestId);

    // 简化回调处理
    void HandleSingleResourceWithCallback(FString RequestId);
    void HandleFolderResourcesWithCallback(FString RequestId);

    // 静态回调处理
    void HandleSingleResourceWithStaticCallback(FString RequestId);
    void HandleFolderResourcesWithStaticCallback(FString RequestId);

    // 内部静态回调方法
    void InternalLoadResourceAsyncWithStaticCallback(const FString& ResourcePath, const FOnResourceLoadedStaticDelegate& Callback);
    void InternalLoadResourceAsyncByClassWithStaticCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedStaticDelegate& Callback);
    void InternalLoadResourcesInFolderAsyncWithStaticCallback(const FString& FolderPath, const FOnResourcesLoadedStaticDelegate& Callback);
    void InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedStaticDelegate& Callback);

    // 工具方法
    FString SanitizeResourcePath(const FString& ResourcePath) const;
    bool IsValidResourceClass(TSubclassOf<UObject> ResourceClass) const;
    FString GenerateRequestId() const;

    // 内部方法
    void BuildLookupTables();
    bool GetResourceTableRow(const FName& ResourceID, FResourceTableRow& OutRow) const;
};