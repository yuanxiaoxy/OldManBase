// Fill out your copyright notice in the Description page of Project Settings.

#include "ResourceManager/ResourceManager.h"
#include "Engine/Engine.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "TimerManager.h"

// ��̬ʵ������
template<>
UResourceManager* TSingleton<UResourceManager>::SingletonInstance = nullptr;

UResourceManager::UResourceManager()
{
    // ���캯��
}

UResourceManager::~UResourceManager()
{
    // ���������첽���ؾ��
    for (auto& HandlePair : AsyncHandles)
    {
        if (HandlePair.Value.IsValid())
        {
            HandlePair.Value->CancelHandle();
        }
    }
    AsyncHandles.Empty();

    // ��ջص�ӳ��
    SingleResourceCallbacks.Empty();
    MultiResourceCallbacks.Empty();
    SingleResourceStaticCallbacks.Empty();
    MultiResourceStaticCallbacks.Empty();

    // ��ջ��棨����ж����Դ����UE�������մ���
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

// ========== ������Դ���� ==========

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

// ========== �첽��Դ������ʹ������ID�� ==========

FString UResourceManager::LoadResourceAsync(const FString& ResourcePath)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // ��黺��
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // �������
        SingleResourceResults.Add(RequestId, CachedResource);
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // ��һ֡����ί��
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourceFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // ʹ��lambda����RequestId������ί�в�������
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
    // ��Ϊֱ�Ӽ��أ����ͼ���ڼ��غ���
    return LoadResourceAsync(ResourcePath);
}

FString UResourceManager::LoadResourcesInFolderAsync(const FString& FolderPath)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // ��ȡ�ļ�����������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // �������
        MultiResourceResults.Add(RequestId, TArray<UObject*>());
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // ��һ֡����ί��
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourcesFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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

    // ��ȡ�ļ�����ָ�����͵�������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // �������
        MultiResourceResults.Add(RequestId, TArray<UObject*>());
        AsyncRequests.Add(RequestId, FAsyncLoadRequest(RequestId, SanitizedPath));
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // ��һ֡����ί��
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, RequestId]() {
                OnResourcesFinishLoaded.Broadcast(RequestId);
                }, 0.1f, false);
        }

        return RequestId;
    }

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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

// ========== �򻯵��첽��Դ������ֱ�Ӱ󶨻ص��� ==========

void UResourceManager::LoadResourceAsyncWithCallback(const FString& ResourcePath, const FOnResourceLoadedCallback& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // ��黺��
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // ����ִ�лص�
        if (Callback.IsBound())
        {
            Callback.Execute(CachedResource);
        }
        return;
    }

    // �洢�ص�
    SingleResourceCallbacks.Add(RequestId, Callback);

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص�������nullptr��
        if (Callback.IsBound())
        {
            Callback.Execute(nullptr);
        }
        SingleResourceCallbacks.Remove(RequestId);
    }
}

void UResourceManager::LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedCallback& Callback)
{
    // ��Ϊֱ�Ӽ��أ����ͼ���ڼ��غ���
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
        // ����ִ�лص���ʾʧ��
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
        // ʹ�����е��ļ��м����߼�����Ҫ����������
        // ����򻯴���ʵ��Ӧ�ô���һ���µ��ڲ�����
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

                    // ��ӵ�����
                    for (UObject* Resource : LoadedResources)
                    {
                        if (Resource)
                        {
                            AddToCache(Resource->GetPathName(), Resource);
                        }
                    }

                    // ִ�лص�
                    if (Callback.IsBound())
                    {
                        Callback.Execute(LoadedResources);
                    }

                    // ����
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
        // ����ִ�пջص�
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

    // ��ȡ�ļ�����������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // ����ִ�лص�
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        return;
    }

    // �洢�ص�
    MultiResourceCallbacks.Add(RequestId, Callback);

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص������ݿ����飩
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

    // ��ȡ�ļ�����ָ�����͵�������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // ����ִ�лص�
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        return;
    }

    // �洢�ص�
    MultiResourceCallbacks.Add(RequestId, Callback);

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص������ݿ����飩
        if (Callback.IsBound())
        {
            Callback.Execute(TArray<UObject*>());
        }
        MultiResourceCallbacks.Remove(RequestId);
    }
}

// ========== �ڲ���̬�ص����� ==========

void UResourceManager::InternalLoadResourceAsyncWithStaticCallback(const FString& ResourcePath, const FOnResourceLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // ��黺��
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        // ����ִ�лص�
        Callback.ExecuteIfBound(CachedResource);
        return;
    }

    // �洢��̬�ص�
    SingleResourceStaticCallbacks.Add(RequestId, Callback);

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص�������nullptr��
        Callback.ExecuteIfBound(nullptr);
        SingleResourceStaticCallbacks.Remove(RequestId);
    }
}

void UResourceManager::InternalLoadResourceAsyncByClassWithStaticCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedStaticDelegate& Callback)
{
    // ��Ϊֱ�Ӽ��أ����ͼ���ڼ��غ���
    InternalLoadResourceAsyncWithStaticCallback(ResourcePath, Callback);
}

void UResourceManager::InternalLoadResourcesInFolderAsyncWithStaticCallback(const FString& FolderPath, const FOnResourcesLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // ��ȡ�ļ�����������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolder(SanitizedPath);

    if (ResourcePaths.Num() == 0)
    {
        // ����ִ�лص�
        Callback.ExecuteIfBound(TArray<UObject*>());
        return;
    }

    // �洢��̬�ص�
    MultiResourceStaticCallbacks.Add(RequestId, Callback);

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص������ݿ����飩
        Callback.ExecuteIfBound(TArray<UObject*>());
        MultiResourceStaticCallbacks.Remove(RequestId);
    }
}

void UResourceManager::InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedStaticDelegate& Callback)
{
    FString RequestId = GenerateRequestId();
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // ��ȡ�ļ�����ָ�����͵�������Դ·��
    TArray<FString> ResourcePaths = GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass);

    if (ResourcePaths.Num() == 0)
    {
        // ����ִ�лص�
        Callback.ExecuteIfBound(TArray<UObject*>());
        return;
    }

    // �洢��̬�ص�
    MultiResourceStaticCallbacks.Add(RequestId, Callback);

    // ���������·������
    TArray<FSoftObjectPath> SoftObjectPaths;
    for (const FString& Path : ResourcePaths)
    {
        SoftObjectPaths.Add(FSoftObjectPath(Path));
    }

    // ʹ��lambda����RequestId
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
        // ����ʧ�ܣ�ִ�лص������ݿ����飩
        Callback.ExecuteIfBound(TArray<UObject*>());
        MultiResourceStaticCallbacks.Remove(RequestId);
    }
}

// ========== �첽������� ==========

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

// ========== ��Դ������� ==========

void UResourceManager::PreloadMarkedResources()
{
    if (!ResourceDataTable)
        return;

    TArray<FResourceTableRow*> AllRows;
    ResourceDataTable->GetAllRows<FResourceTableRow>(TEXT("ResourceManager"), AllRows);

    // �����ȼ�����
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

// ========== ��Դ��Ϣ��ѯ ==========

FResourceInfo UResourceManager::GetResourceInfo(const FString& ResourcePath)
{
    FResourceInfo Info;
    Info.ResourcePath = ResourcePath;

    FString SanitizedPath = SanitizeResourcePath(ResourcePath);
    Info.LoadedResource = GetFromCache(SanitizedPath);
    Info.LoadState = Info.LoadedResource ? EResourceLoadState::Loaded : EResourceLoadState::NotLoaded;

    // ��ȡ��Դ��
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

    // ȷ���ʲ�ע����Ѿ����ɨ��
    if (!AssetRegistry.IsLoadingAssets())
    {
        TArray<FAssetData> AssetDataList;
        FString SanitizedPath = SanitizeResourcePath(FolderPath);

        // �ݹ������ļ���
        AssetRegistry.GetAssetsByPath(FName(*SanitizedPath), AssetDataList, true);

        for (const FAssetData& AssetData : AssetDataList)
        {
            // ʹ���µ�API��������õ�ObjectPath
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

        // ������������ - ʹ���µ�API
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

// ========== ��Դж�ع��� ==========

void UResourceManager::UnloadResource(const FString& ResourcePath)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // �ӻ����Ƴ�
    RemoveFromCache(SanitizedPath);

    // ȡ���첽����
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

    // ��ȡ�ļ�����������Դ·��
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

// ========== ���Թ��� ==========

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

// ========== �ڲ�ʵ�� ==========

UObject* UResourceManager::InternalLoadResourceSync(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass)
{
    FString SanitizedPath = SanitizeResourcePath(ResourcePath);

    // ��黺��
    if (UObject* CachedResource = GetFromCache(SanitizedPath))
    {
        return CachedResource;
    }

    // ͬ��������Դ
    FSoftObjectPath SoftObjectPath(SanitizedPath);
    UObject* LoadedResource = StreamableManager.LoadSynchronous(SoftObjectPath);

    if (LoadedResource)
    {
        // ��ӵ�����
        AddToCache(SanitizedPath, LoadedResource);
    }

    return LoadedResource;
}

TArray<UObject*> UResourceManager::InternalLoadResourcesInFolderSync(const FString& FolderPath, TSubclassOf<UObject> ResourceClass)
{
    TArray<UObject*> LoadedResources;
    FString SanitizedPath = SanitizeResourcePath(FolderPath);

    // ��ȡ��Դ·��
    TArray<FString> ResourcePaths = ResourceClass ?
        GetResourcePathsInFolderByClass(SanitizedPath, ResourceClass) :
        GetResourcePathsInFolder(SanitizedPath);

    // ͬ������������Դ
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

// �����Ļص�����ʵ��
void UResourceManager::HandleSingleResourceLoaded(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // ��ӵ�����
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);

            // �洢���
            SingleResourceResults.Add(RequestId, LoadedResource);
            AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

            // �㲥ί��
            OnResourceFinishLoaded.Broadcast(RequestId);
        }
        else
        {
            AsyncRequests[RequestId].LoadState = EResourceLoadState::Failed;
        }

        // ������
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

        // ��ӵ�����
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // �洢���
        MultiResourceResults.Add(RequestId, LoadedResources);
        AsyncRequests[RequestId].LoadState = EResourceLoadState::Loaded;

        // �㲥ί��
        OnResourcesFinishLoaded.Broadcast(RequestId);

        // ������
        AsyncHandles.Remove(RequestId);
    }
}

// �򻯻ص�����
void UResourceManager::HandleSingleResourceWithCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // ��ӵ�����
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);
        }

        // ִ�лص�
        FOnResourceLoadedCallback* Callback = SingleResourceCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResource);
        }

        // ����
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

        // ��ӵ�����
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // ִ�лص�
        FOnResourcesLoadedCallback* Callback = MultiResourceCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResources);
        }

        // ����
        MultiResourceCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

// ��̬�ص�����
void UResourceManager::HandleSingleResourceWithStaticCallback(FString RequestId)
{
    TSharedPtr<FStreamableHandle>* HandlePtr = AsyncHandles.Find(RequestId);
    if (HandlePtr && HandlePtr->IsValid() && AsyncRequests.Contains(RequestId))
    {
        UObject* LoadedResource = (*HandlePtr)->GetLoadedAsset();

        if (LoadedResource)
        {
            // ��ӵ�����
            FString ResourcePath = AsyncRequests[RequestId].ResourcePath;
            AddToCache(ResourcePath, LoadedResource);
        }

        // ִ�о�̬�ص�
        FOnResourceLoadedStaticDelegate* Callback = SingleResourceStaticCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResource);
        }

        // ����
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

        // ��ӵ�����
        for (UObject* Resource : LoadedResources)
        {
            if (Resource)
            {
                AddToCache(Resource->GetPathName(), Resource);
            }
        }

        // ִ�о�̬�ص�
        FOnResourcesLoadedStaticDelegate* Callback = MultiResourceStaticCallbacks.Find(RequestId);
        if (Callback && Callback->IsBound())
        {
            Callback->Execute(LoadedResources);
        }

        // ����
        MultiResourceStaticCallbacks.Remove(RequestId);
        AsyncHandles.Remove(RequestId);
        AsyncRequests.Remove(RequestId);
    }
}

FString UResourceManager::SanitizeResourcePath(const FString& ResourcePath) const
{
    FString SanitizedPath = ResourcePath;

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

    // �������ұ�
    TArray<FResourceTableRow*> AllRows;
    ResourceDataTable->GetAllRows<FResourceTableRow>(TEXT("ResourceManager"), AllRows);

    for (const FResourceTableRow* Row : AllRows)
    {
        if (Row && !Row->ResourceID.IsNone() && !Row->ResourcePath.IsEmpty())
        {
            // ��ӵ�ID��·����ӳ��
            ResourceIDToPathMap.Add(Row->ResourceID, Row->ResourcePath);

            // ��ӵ�����ӳ��
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
