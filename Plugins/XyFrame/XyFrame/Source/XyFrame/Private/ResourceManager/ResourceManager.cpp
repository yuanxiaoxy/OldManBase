// Fill out your copyright notice in the Description page of Project Settings.

#include "ResourceManager/ResourceManager.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "TimerManager.h"

// 静态实例定义
template<>
UResourceManager* TSingleton<UResourceManager>::SingletonInstance = nullptr;

UResourceManager::UResourceManager()
{
    // 构造函数
}

UResourceManager::~UResourceManager()
{
    // 清理所有异步加载句柄
    for (auto& HandlePair : AsyncHandles)
    {
        if (HandlePair.Value.IsValid())
        {
            HandlePair.Value->CancelHandle();
        }
    }
    AsyncHandles.Empty();

    // 清空回调映射
    SingleResourceCallbacks.Empty();
    MultiResourceCallbacks.Empty();
    SingleResourceStaticCallbacks.Empty();
    MultiResourceStaticCallbacks.Empty();

    // 清空缓存（但不卸载资源，由UE垃圾回收处理）
    ResourceCache.Empty();
}

void UResourceManager::SetResourceDataTable(UDataTable* InResourceDataTable)
{
    ResourceDataTable = InResourceDataTable;
    BuildLookupTables();

    UE_LOG(LogTemp, Log, TEXT("Resource DataTable set, loaded %d resources"), ResourceIDToPathMap.Num());
}

void UResourceManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("ResourceManager InitializeSingleton called"));
    InitializeResourceManager();
}

void UResourceManager::InitializeResourceManager()
{
    UE_LOG(LogTemp, Log, TEXT("Resource Manager Initialized"));
}

// ========== 基础资源操作 ==========

UObject* UResourceManager::LoadResourceSync(const FString& ResourcePath)
{
    return InternalLoadResourceSync(ResourcePath);
}

UObject* UResourceManager::LoadResourceByID(const FName& ResourceID)
{
    FString ResourcePath = GetResourcePathByID(ResourceID);
    if (!ResourcePath.IsEmpty())
    {
        return LoadResourceSync(ResourcePath);
    }
    return nullptr;
}

UObject* UResourceManager::LoadResourceSyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass)
{
    return InternalLoadResourceSync(ResourcePath, ResourceClass);
}

TArray<UObject*> UResourceManager::LoadResourcesInFolderSync(const FString& FolderPath)
{
    return InternalLoadResourcesInFolderSync(FolderPath);
}

TArray<UObject*> UResourceManager::LoadResourcesInFolderSyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass)
{
    return InternalLoadResourcesInFolderSync(FolderPath, ResourceClass);
}

// ========== 异步资源操作（使用请求ID） ==========

FString UResourceManager::LoadResourceAsync(const FString& ResourcePath)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // 检查缓存
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // 立即完成
        SingleResourceResults.Add(RequestId, CachedResource);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // 下一帧触发委托
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourceFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // 使用lambda捕获RequestId，避免委托参数问题
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        FSoftObjectPath(SanitizedPath),
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleSingleResourceLoaded(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;

        return RequestId;
    }

    return FString();
}

FString UResourceManager::LoadResourceAsyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass)
{
    // 简化为直接加载，类型检查在加载后处理
    return LoadResourceAsync(ResourcePath);
}

FString UResourceManager::LoadResourcesInFolderAsync(const FString& FolderPath)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // 立即完成
        MultiResourceResults.Add(RequestId, TArray<UObject*>());
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // 下一帧触发委托
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourcesFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesLoaded(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;

        return RequestId;
    }

    return FString();
}

FString UResourceManager::LoadResourcesInFolderAsyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下指定类型的所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // 立即完成
        MultiResourceResults.Add(RequestId, TArray<UObject*>());
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // 下一帧触发委托
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourcesFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesLoaded(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;

        return RequestId;
    }

    return FString();
}

// ========== 简化的异步资源操作（直接绑定回调） ==========

void UResourceManager::LoadResourceAsyncWithCallback(const FString& ResourcePath, const FOnResourceLoadedCallback& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // 检查缓存
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // 立即执行回调
        if (Callback.IsBound())
        {
            Callback.Execute(CachedResource);
        }
        return;
    }

    // 存储回调
    SingleResourceCallbacks.Add(RequestId, Callback);

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        FSoftObjectPath(SanitizedPath),
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleSingleResourceWithCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递nullptr）
        if (Callback.IsBound())
        {
            Callback.Execute(nullptr);
        }
        SingleResourceCallbacks.Remove(RequestId);
    }
}

void UResourceManager::LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedCallback& Callback)
{
    // 简化为直接加载，类型检查在加载后处理
    LoadResourceAsyncWithCallback(ResourcePath, Callback);
}

FString UResourceManager::LoadResourceByIDAsync(const FName& ResourceID)
{
    FString ResourcePath = GetResourcePathByID(ResourceID);
    if (!ResourcePath.IsEmpty())
    {
        return LoadResourceAsync(ResourcePath);
    }
    return FString();
}

void UResourceManager::LoadResourceByIDWithCallback(const FName& ResourceID, const FOnResourceLoadedCallback& Callback)
{
    FString ResourcePath = GetResourcePathByID(ResourceID);
    if (!ResourcePath.IsEmpty())
    {
        LoadResourceAsyncWithCallback(ResourcePath, Callback);
    }
    else
    {
        // 立即执行回调表示失败
        if (Callback.IsBound())
        {
            Callback.Execute(nullptr);
        }
    }
}

void UResourceManager::LoadResourcesByCategory(EResourceCategory Category, const FOnResourcesLoadedCallback& Callback)
{
    TArray<FName> ResourceIDs = GetResourceIDsByCategory(Category);
    TArray<FString> ResourcePaths;

    for (const FName& ResourceID : ResourceIDs)
    {
        FString ResourcePath = GetResourcePathByID(ResourceID);
        if (!ResourcePath.IsEmpty())
        {
            ResourcePaths.Add(ResourcePath);
        }
    }

    if (ResourcePaths.Num() > 0)
    {
        // 使用现有的文件夹加载逻辑（需要稍作调整）
        // 这里简化处理，实际应该创建一个新的内部方法
        FString RequestId = GenerateRequestId();

        TArray<FSoftObjectPath> SoftObjectPaths;
        for (const FString& Path : ResourcePaths)
        {
            SoftObjectPaths.Add(FSoftObjectPath(Path));
        }

        TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
            SoftObjectPaths,
            FStreamableDelegate::CreateWeakLambda(this, [this, RequestId, Callback]() {
                TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
                if (HandlePtr && HandlePtr->IsValid())
                {
                    TArray<UObject*> LoadedResources;
                    (*HandlePtr)->GetLoadedAssets(LoadedResources);

                    // 添加到缓存
                    for (UObject* Resource : LoadedResources)
                    {
                        if (Resource)
                        {
                            AddToCache(Resource->GetPathName(), Resource);
                        }
                    }

                    // 执行回调
                    if (Callback.IsBound())
                    {
                        Callback.Execute(LoadedResources);
                    }

                    // 清理
                    AsyncHandles.Remove(RequestId);
                }
                })
        );

        if (Handle.IsValid())
        {
            AsyncHandles.Add(RequestId, Handle);
        }
    }
    else
    {
        // 立即执行空回调
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
    }
}

void UResourceManager::LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, const FOnResourcesLoadedCallback& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // 立即执行回调
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        return;
    }

    // 存储回调
    MultiResourceCallbacks.Add(RequestId, Callback);

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesWithCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递空数组）
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        MultiResourceCallbacks.Remove(RequestId);
    }
}

void UResourceManager::LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedCallback& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下指定类型的所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // 立即执行回调
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        return;
    }

    // 存储回调
    MultiResourceCallbacks.Add(RequestId, Callback);

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesWithCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递空数组）
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        MultiResourceCallbacks.Remove(RequestId);
    }
}

// ========== 内部静态回调方法 ==========

void UResourceManager::InternalLoadResourceAsyncWithStaticCallback(const FString& ResourcePath, const FOnResourceLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // 检查缓存
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // 立即执行回调
        Callback.ExecuteIfBound(CachedResource);
        return;
    }

    // 存储静态回调
    SingleResourceStaticCallbacks.Add(RequestId, Callback);

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        FSoftObjectPath(SanitizedPath),
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleSingleResourceWithStaticCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递nullptr）
        Callback.ExecuteIfBound(nullptr);
        SingleResourceStaticCallbacks.Remove(RequestId);
    }
}

void UResourceManager::InternalLoadResourceAsyncByClassWithStaticCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedStaticDelegate& Callback)
{
    // 简化为直接加载，类型检查在加载后处理
    InternalLoadResourceAsyncWithStaticCallback(ResourcePath, Callback);
}

void UResourceManager::InternalLoadResourcesInFolderAsyncWithStaticCallback(const FString& FolderPath, const FOnResourcesLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // 立即执行回调
        Callback.ExecuteIfBound(TArray<UObject*>());
        return;
    }

    // 存储静态回调
    MultiResourceStaticCallbacks.Add(RequestId, Callback);

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesWithStaticCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递空数组）
        Callback.ExecuteIfBound(TArray<UObject*>());
        MultiResourceStaticCallbacks.Remove(RequestId);
    }
}

void UResourceManager::InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下指定类型的所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // 立即执行回调
        Callback.ExecuteIfBound(TArray<UObject*>());
        return;
    }

    // 存储静态回调
    MultiResourceStaticCallbacks.Add(RequestId, Callback);

    // 创建软对象路径数组
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // 使用lambda捕获RequestId
    TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
        SoftObjectPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId]() {
            HandleFolderResourcesWithStaticCallback(RequestId);
            })
    );

    if (Handle.IsValid())
    {
        AsyncHandles.Add(RequestId, Handle);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loading;
    }
    else
    {
        // 加载失败，执行回调（传递空数组）
        Callback.ExecuteIfBound(TArray<UObject*>());
        MultiResourceStaticCallbacks.Remove(RequestId);
    }
}

// ========== 异步请求管理 ==========

EResourceLoadState UResourceManager::GetAsyncRequestState(const FString& RequestId) const
{
    const FAsyncLoadRequest* Request = AsyncRequests.Find(RequestId);
    return Request ? Request->LoadState : EResourceLoadState::Failed;
}

UObject* UResourceManager::GetAsyncRequestResource(const FString& RequestId) const
{
    UObject* const* Result = SingleResourceResults.Find(RequestId);
    return Result ? *Result : nullptr;
}

TArray<UObject*> UResourceManager::GetAsyncRequestResources(const FString& RequestId) const
{
    const TArray<UObject*>* Result = MultiResourceResults.Find(RequestId);
    return Result ? *Result : TArray<UObject*>();
}

void UResourceManager::CancelAsyncRequest(const FString& RequestId)
{
    TSharedPtr<FStreamableHandle> Handle = AsyncHandles.FindRef(RequestId);
    if (Handle.IsValid())
    {
        Handle->CancelHandle();
    }

    AsyncHandles.Remove(RequestId);
    AsyncRequests.Remove(RequestId);
    SingleResourceResults.Remove(RequestId);
    MultiResourceResults.Remove(RequestId);
    SingleResourceCallbacks.Remove(RequestId);
    MultiResourceCallbacks.Remove(RequestId);
    SingleResourceStaticCallbacks.Remove(RequestId);
    MultiResourceStaticCallbacks.Remove(RequestId);
}

// ========== 资源缓存管理 ==========

void UResourceManager::PreloadMarkedResources()
{
    if (!ResourceDataTable)
        return;

    TArray<FResourceTableRow*> AllRows;
    ResourceDataTable->GetAllRows<FResourceTableRow>(TEXT("ResourceManager"), AllRows);

    // 按优先级排序
    AllRows.Sort([](const FResourceTableRow& A, const FResourceTableRow& B) {
        return A.LoadPriority < B.LoadPriority;
        });

    int32 PreloadCount = 0;
    for (const FResourceTableRow* Row : AllRows)
    {
        if (Row && Row->bPreload && !Row->ResourcePath.IsEmpty())
        {
            LoadResourceAsync(Row->ResourcePath);
            PreloadCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Preloaded %d marked resources"), PreloadCount);
}

void UResourceManager::AddToCache(const FString& ResourcePath, UObject* Resource)
{
    if (Resource)
    {
        FString SanitizedPath = SanitizeResourcePath(ResourcePath);
        ResourceCache.Add(SanitizedPath, Resource);
    }
}

UObject* UResourceManager::GetFromCache(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    UObject** CachedResource = ResourceCache.Find(SanitizedPath);
    return CachedResource ? *CachedResource : nullptr;
}

bool UResourceManager::IsInCache(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    return ResourceCache.Contains(SanitizedPath);
}

void UResourceManager::RemoveFromCache(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    ResourceCache.Remove(SanitizedPath);
}

void UResourceManager::ClearCache()
{
    ResourceCache.Empty();
}

int32 UResourceManager::GetCacheSize() const
{
    return ResourceCache.Num();
}

// ========== 资源信息查询 ==========

FResourceInfo UResourceManager::GetResourceInfo(const FString& ResourcePath)
{
    FResourceInfo Info;
    Info.ResourcePath = ResourcePath;

    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    Info.LoadedResource = GetFromCache(SanitizedPath);
    Info.LoadState = Info.LoadedResource ? EResourceLoadState::Loaded : EResourceLoadState::NotLoaded;

    // 提取资源名
    FString CleanPath = SanitizedPath;
    int32 LastSlash = 0;
    if (CleanPath.FindLastChar('/', LastSlash))
    {
        Info.ResourceName = CleanPath.RightChop(LastSlash + 1);
    }
    else
    {
        Info.ResourceName = CleanPath;
    }

    return Info;
}

bool UResourceManager::GetResourceInfoByID(const FName& ResourceID, FResourceTableRow& OutInfo) const
{
    return GetResourceTableRow(ResourceID, OutInfo);
}

bool UResourceManager::DoesResourceExist(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    FSoftObjectPath SoftObjectPath(SanitizedPath);
    return SoftObjectPath.IsValid();
}

bool UResourceManager::DoesResourceIDExist(const FName& ResourceID) const
{
    return ResourceIDToPathMap.Contains(ResourceID);
}

TArray<FString> UResourceManager::GetResourcePathsInFolder(const FString& FolderPath)
{
    TArray<FString> ResourcePaths;

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    // 确保资产注册表已经完成扫描
    if (!AssetRegistry.IsLoadingAssets())
    {
        TArray<FAssetData> AssetDataList;
        FString SanitizedPath = SanitizeResourcePath(FolderPath);

        // 递归搜索文件夹
        AssetRegistry.GetAssetsByPath(FName(*SanitizedPath), AssetDataList, true);

        for (const FAssetData& AssetData : AssetDataList)
        {
            // 使用新的API替代已弃用的ObjectPath
            ResourcePaths.Add(AssetData.GetObjectPathString());
        }
    }

    return ResourcePaths;
}

TArray<FString> UResourceManager::GetResourcePathsInFolderByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass)
{
    TArray<FString> ResourcePaths;

    if (!IsValidResourceClass(ResourceClass))
    {
        return ResourcePaths;
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    if (!AssetRegistry.IsLoadingAssets())
    {
        TArray<FAssetData> AssetDataList;
        FString SanitizedPath = SanitizeResourcePath(FolderPath);

        // 根据类名过滤 - 使用新的API
        FTopLevelAssetPath ClassName = ResourceClass->GetClassPathName();
        AssetRegistry.GetAssetsByClass(ClassName, AssetDataList);

        for (const FAssetData& AssetData : AssetDataList)
        {
            FString AssetPath = AssetData.GetObjectPathString();
            if (AssetPath.StartsWith(SanitizedPath))
            {
                ResourcePaths.Add(AssetPath);
            }
        }
    }

    return ResourcePaths;
}

TArray<FName> UResourceManager::GetResourceIDsByCategory(EResourceCategory Category) const
{
    const TArray<FName>* ResourceIDs = CategoryToResourceIDsMap.Find(Category);
    return ResourceIDs ? *ResourceIDs : TArray<FName>();
}

FString UResourceManager::GetResourcePathByID(const FName& ResourceID) const
{
    const FString* Path = ResourceIDToPathMap.Find(ResourceID);
    return Path ? *Path : FString();
}

// ========== 资源卸载管理 ==========

void UResourceManager::UnloadResource(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // 从缓存移除
    RemoveFromCache(SanitizedPath);

    // 取消异步加载
    for (auto& HandlePair : AsyncHandles)
    {
        if (HandlePair.Value.IsValid() && AsyncRequests[HandlePair.Key].ResourcePath == SanitizedPath)
        {
            HandlePair.Value->CancelHandle();
            AsyncHandles.Remove(HandlePair.Key);
            break;
        }
    }
}

void UResourceManager::UnloadResourcesInFolder(const FString& FolderPath)
{
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取文件夹下所有资源路径
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    for (const FString& ResourcePath : ResourcePaths)
    {
        UnloadResource(ResourcePath);
    }
}

void UResourceManager::ForceGarbageCollection()
{
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
}

// ========== 调试工具 ==========

void UResourceManager::PrintCacheInfo()
{
    UE_LOG(LogTemp, Log, TEXT("=== Resource Cache Info ==="));
    UE_LOG(LogTemp, Log, TEXT("Cache Size: %d"), ResourceCache.Num());

    for (const auto& Pair : ResourceCache)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s -> %s"), *Pair.Key, *GetNameSafe(Pair.Value));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Cache Info ==="));
}

void UResourceManager::PrintFolderResources(const FString& FolderPath)
{
    FString SanitizedPath = SanitizeResourcePath(FolderPath);
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    UE_LOG(LogTemp, Log, TEXT("=== Resources in %s ==="), *SanitizedPath);
    UE_LOG(LogTemp, Log, TEXT("Resource Count: %d"), ResourcePaths.Num());

    for (const FString& Path : ResourcePaths)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s"), *Path);
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Resources ==="));
}

void UResourceManager::PrintAsyncRequests()
{
    UE_LOG(LogTemp, Log, TEXT("=== Async Requests ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Requests: %d"), AsyncRequests.Num());

    for (const auto& Pair : AsyncRequests)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %s -> %s"),
            *Pair.Key,
            *Pair.Value.ResourcePath,
            *UEnum::GetValueAsString(Pair.Value.LoadState));
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Async Requests ==="));
}

// ========== 内部实现 ==========

UObject* UResourceManager::InternalLoadResourceSync(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // 检查缓存
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        return CachedResource;
    }

    // 同步加载资源
    FSoftObjectPath SoftObjectPath(SanitizedPath);
    UObject* LoadedResource = StreamableManager.LoadSynchronous(SoftObjectPath);

    if (LoadedResource)
    {
        // 添加到缓存
        AddToCache(SanitizedPath, LoadedResource);
    }

    return LoadedResource;
}

TArray<UObject*> UResourceManager::InternalLoadResourcesInFolderSync(const FString& FolderPath, TSubclassOf<UObject> ResourceClass)
{
    TArray<UObject*> LoadedResources;
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // 获取资源路径
    TArray<FString> ResourcePaths = ResourceClass ?
        GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass) :
        GetResourcePathsInFolder(SanitizedPath);

    // 同步加载所有资源
    for (const FString& ResourcePath : ResourcePaths)
    {
        UObject* LoadedResource = InternalLoadResourceSync(ResourcePath, ResourceClass);
        if (LoadedResource)
        {
            LoadedResources.Add(LoadedResource);
        }
    }

    return LoadedResources;
}

// 修正的回调函数实现
void UResourceManager::HandleSingleResourceLoaded(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // 添加到缓存
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);

            // 存储结果
            SingleResourceResults.Add(RequestId, LoadedResource);
            AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

            // 广播委托
            OnResourceFinishLoaded.Broadcast(RequestId);
        }
        else
        {
            AsyncRequests[RequestId].LoadState = EResourceLoadState::Failed;
        }

        // 清理句柄
        AsyncHandles.Remove(RequestId);
    }
}

void UResourceManager::HandleFolderResourcesLoaded(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        TArray<UObject*> LoadedResources;
        (*HandlePtr)->GetLoadedAssets(LoadedResources);

        // 添加到缓存
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // 存储结果
        MultiResourceResults.Add(RequestId, LoadedResources);
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // 广播委托
        OnResourcesFinishLoaded.Broadcast(RequestId);

        // 清理句柄
        AsyncHandles.Remove(RequestId);
    }
}

// 简化回调处理
void UResourceManager::HandleSingleResourceWithCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // 添加到缓存
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);
        }

        // 执行回调
        FOnResourceLoadedCallback* Callback = SingleResourceCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResource);
        }

        // 清理
        SingleResourceCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

void UResourceManager::HandleFolderResourcesWithCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        TArray<UObject*> LoadedResources;
        (*HandlePtr)->GetLoadedAssets(LoadedResources);

        // 添加到缓存
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // 执行回调
        FOnResourcesLoadedCallback* Callback = MultiResourceCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResources);
        }

        // 清理
        MultiResourceCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

// 静态回调处理
void UResourceManager::HandleSingleResourceWithStaticCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // 添加到缓存
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);
        }

        // 执行静态回调
        FOnResourceLoadedStaticDelegate* Callback = SingleResourceStaticCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResource);
        }

        // 清理
        SingleResourceStaticCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

void UResourceManager::HandleFolderResourcesWithStaticCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        TArray<UObject*> LoadedResources;
        (*HandlePtr)->GetLoadedAssets(LoadedResources);

        // 添加到缓存
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // 执行静态回调
        FOnResourcesLoadedStaticDelegate* Callback = MultiResourceStaticCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResources);
        }

        // 清理
        MultiResourceStaticCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

FString UResourceManager::SanitizeResourcePath(const FString& ResourcePath) const
{
    FString SanitizedPath = ResourcePath;

    // 移除多余的前缀和后缀
    SanitizedPath.RemoveFromStart(TEXT("/Game/"));
    SanitizedPath.RemoveFromEnd(TEXT("."));

    // 确保路径以游戏内容根目录开始
    if (!SanitizedPath.StartsWith(TEXT("/Game/")))
    {
        SanitizedPath = TEXT("/Game/") + SanitizedPath;
    }

    return SanitizedPath;
}

bool UResourceManager::IsValidResourceClass(TSubclassOf<UObject> ResourceClass) const
{
    return ResourceClass != nullptr && ResourceClass != UObject::StaticClass();
}

FString UResourceManager::GenerateRequestId() const
{
    return FGuid::NewGuid().ToString();
}

void UResourceManager::BuildLookupTables()
{
    ResourceIDToPathMap.Empty();
    CategoryToResourceIDsMap.Empty();

    if (!ResourceDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Resource DataTable is null"));
        return;
    }

    // 构建查找表
    TArray<FResourceTableRow*> AllRows;
    ResourceDataTable->GetAllRows<FResourceTableRow>(TEXT("ResourceManager"), AllRows);

    for (const FResourceTableRow* Row : AllRows)
    {
        if (Row && !Row->ResourceID.IsNone() && !Row->ResourcePath.IsEmpty())
        {
            // 添加到ID到路径的映射
            ResourceIDToPathMap.Add(Row->ResourceID, Row->ResourcePath);

            // 添加到分类映射
            if (!CategoryToResourceIDsMap.Contains(Row->Category))
            {
                CategoryToResourceIDsMap.Add(Row->Category, TArray<FName>());
            }
            CategoryToResourceIDsMap[Row->Category].Add(Row->ResourceID);

            UE_LOG(LogTemp, Verbose, TEXT("Registered resource: %s -> %s (Category: %s)"),
                *Row->ResourceID.ToString(), *Row->ResourcePath, *UEnum::GetValueAsString(Row->Category));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Built lookup tables: %d resources, %d categories"),
        ResourceIDToPathMap.Num(), CategoryToResourceIDsMap.Num());
}

bool UResourceManager::GetResourceTableRow(const FName& ResourceID, FResourceTableRow& OutRow) const
{
    if (!ResourceDataTable)
        return false;

    FResourceTableRow* Row = ResourceDataTable->FindRow<FResourceTableRow>(ResourceID, TEXT("ResourceManager"));
    if (Row)
    {
        OutRow = *Row;
        return true;
    }
    return false;
}
