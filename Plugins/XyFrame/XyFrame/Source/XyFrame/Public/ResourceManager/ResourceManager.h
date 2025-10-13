// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "ResourceTableRow.h"
#include "ResourceManager.generated.h"

// ��Դ����״̬
UENUM(BlueprintType)
enum class EResourceLoadState : uint8
{
    NotLoaded UMETA(DisplayName = "Not Loaded"),
    Loading UMETA(DisplayName = "Loading"),
    Loaded UMETA(DisplayName = "Loaded"),
    Failed UMETA(DisplayName = "Failed")
};

// ��Դ��Ϣ�ṹ
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

// �첽��������ID
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

// ��Դ�������ί��
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceFinishLoadedSignature, const FString&, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourcesFinishLoadedSignature, const FString&, RequestId);

// �µļ򻯻ص�ί�� - ֱ�Ӵ��ݼ��ص���Դ
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnResourceLoadedCallback, UObject*, LoadedResource);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnResourcesLoadedCallback, const TArray<UObject*>&, LoadedResources);

// ��̬ί������C++��Ա����ָ��
DECLARE_DELEGATE_OneParam(FOnResourceLoadedStaticDelegate, UObject*);
DECLARE_DELEGATE_OneParam(FOnResourcesLoadedStaticDelegate, const TArray<UObject*>&);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UResourceManager : public USingletonBase
{
    GENERATED_BODY()

    // ��������
    DECLARE_SINGLETON(UResourceManager)

public:
    // ��ʼ����Դ������
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void InitializeResourceManager();

    // ��д��ʼ������
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); };

    // ��ȡʵ������ͼ�ɵ��÷���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Resource", meta = (DisplayName = "Get Resource Manager"))
    static UResourceManager* GetResourceManager() { return GetInstance(); }

    // ������Դ���ݱ�
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void SetResourceDataTable(UDataTable* InResourceDataTable);


    // Ĭ�Ϲ��캯��
    UResourceManager();

    // ��������
    virtual ~UResourceManager() override;

    // ========== ��ͼ�ɷ���ί�� ==========

    // ������Դ�������ί��
    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourceFinishLoadedSignature OnResourceFinishLoaded;

    // �����Դ�������ί��
    UPROPERTY(BlueprintAssignable, Category = "Resource")
    FOnResourcesFinishLoadedSignature OnResourcesFinishLoaded;

    // ========== ������Դ���� ==========

    // ͬ�����ص�����Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* LoadResourceSync(const FString& ResourcePath);

    // ͨ����ԴID������Դ��ͬ����
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    UObject* LoadResourceByID(const FName& ResourceID);

    // ͬ������ָ�����͵ĵ�����Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* LoadResourceSyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass);

    // ͬ�������ļ�����������Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> LoadResourcesInFolderSync(const FString& FolderPath);

    // ͬ�������ļ�����ָ�����͵�������Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> LoadResourcesInFolderSyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // ========== �첽��Դ������ʹ������ID�� ==========

    // �첽���ص�����Դ - ��������ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourceAsync(const FString& ResourcePath);

    // �첽����ָ�����͵ĵ�����Դ - ��������ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourceAsyncByClass(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass);

    // �첽�����ļ�����������Դ - ��������ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourcesInFolderAsync(const FString& FolderPath);

    // �첽�����ļ�����ָ�����͵�������Դ - ��������ID
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FString LoadResourcesInFolderAsyncByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // ========== �򻯵��첽��Դ������ֱ�Ӱ󶨻ص��� ==========

    // �첽���ص�����Դ - ֱ�Ӱ󶨻ص�����ͼ��
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resource Async With Callback"))
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, const FOnResourceLoadedCallback& Callback);

    // �첽����ָ�����͵ĵ�����Դ - ֱ�Ӱ󶨻ص�����ͼ��
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resource Async By Class With Callback"))
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedCallback& Callback);

    // ͨ����ԴID������Դ���첽��- ��������ID
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    FString LoadResourceByIDAsync(const FName& ResourceID);

    // ͨ����ԴID������Դ���첽��- ֱ�ӻص�
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void LoadResourceByIDWithCallback(const FName& ResourceID, const FOnResourceLoadedCallback& Callback);

    // ��������ض����Դ
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void LoadResourcesByCategory(EResourceCategory Category, const FOnResourcesLoadedCallback& Callback);

    // �첽�����ļ�����������Դ - ֱ�Ӱ󶨻ص�����ͼ��
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resources In Folder Async With Callback"))
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, const FOnResourcesLoadedCallback& Callback);

    // �첽�����ļ�����ָ�����͵�������Դ - ֱ�Ӱ󶨻ص�����ͼ��
    UFUNCTION(BlueprintCallable, Category = "Resource", meta = (DisplayName = "Load Resources In Folder Async By Class With Callback"))
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedCallback& Callback);

    // ========== C++ģ�巽�� - ֱ�Ӵ��ݳ�Ա����ָ�� ==========

    // �첽���ص�����Դ - ֱ�Ӱ󶨳�Ա����ָ��
    template<typename T>
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, T* Object, void(T::* Function)(UObject*))
    {
        FOnResourceLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourceAsyncWithStaticCallback(ResourcePath, StaticDelegate);
    }

    // �첽���ص�����Դ - �޲γ�Ա����ָ��
    template<typename T>
    void LoadResourceAsyncWithCallback(const FString& ResourcePath, T* Object, void(T::* Function)())
    {
        // ʹ��lambda��װ�޲κ��������Լ��ص���Դ����
        FOnResourceLoadedStaticDelegate StaticDelegate = FOnResourceLoadedStaticDelegate::CreateLambda([Object, Function](UObject* LoadedResource) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourceAsyncWithStaticCallback(ResourcePath, StaticDelegate);
    }

    // �첽����ָ�����͵ĵ�����Դ - ֱ�Ӱ󶨳�Ա����ָ�루��������
    template<typename T>
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)(UObject*))
    {
        FOnResourceLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourceAsyncByClassWithStaticCallback(ResourcePath, ResourceClass, StaticDelegate);
    }

    // �첽����ָ�����͵ĵ�����Դ - �޲γ�Ա����ָ��
    template<typename T>
    void LoadResourceAsyncByClassWithCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)())
    {
        // ʹ��lambda��װ�޲κ��������Լ��ص���Դ����
        FOnResourceLoadedStaticDelegate StaticDelegate = FOnResourceLoadedStaticDelegate::CreateLambda([Object, Function](UObject* LoadedResource) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourceAsyncByClassWithStaticCallback(ResourcePath, ResourceClass, StaticDelegate);
    }

    // ͨ����ԴID������Դ���첽��- ��Ա����ָ��ص�
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

    // �첽�����ļ�����������Դ - ֱ�Ӱ󶨳�Ա����ָ�루��������
    template<typename T>
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, T* Object, void(T::* Function)(const TArray<UObject*>&))
    {
        FOnResourcesLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourcesInFolderAsyncWithStaticCallback(FolderPath, StaticDelegate);
    }

    // �첽�����ļ�����������Դ - �޲γ�Ա����ָ��
    template<typename T>
    void LoadResourcesInFolderAsyncWithCallback(const FString& FolderPath, T* Object, void(T::* Function)())
    {
        // ʹ��lambda��װ�޲κ��������Լ��ص���Դ�������
        FOnResourcesLoadedStaticDelegate StaticDelegate = FOnResourcesLoadedStaticDelegate::CreateLambda([Object, Function](const TArray<UObject*>& LoadedResources) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourcesInFolderAsyncWithStaticCallback(FolderPath, StaticDelegate);
    }

    // �첽�����ļ�����ָ�����͵�������Դ - ֱ�Ӱ󶨳�Ա����ָ�루��������
    template<typename T>
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)(const TArray<UObject*>&))
    {
        FOnResourcesLoadedStaticDelegate StaticDelegate;
        StaticDelegate.BindUObject(Object, Function);
        InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(FolderPath, ResourceClass, StaticDelegate);
    }

    // �첽�����ļ�����ָ�����͵�������Դ - �޲γ�Ա����ָ��
    template<typename T>
    void LoadResourcesInFolderAsyncByClassWithCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, T* Object, void(T::* Function)())
    {
        // ʹ��lambda��װ�޲κ��������Լ��ص���Դ�������
        FOnResourcesLoadedStaticDelegate StaticDelegate = FOnResourcesLoadedStaticDelegate::CreateLambda([Object, Function](const TArray<UObject*>& LoadedResources) {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            });
        InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(FolderPath, ResourceClass, StaticDelegate);
    }

    // ========== �첽������� ==========

    // ��ȡ�첽����״̬
    UFUNCTION(BlueprintCallable, Category = "Resource")
    EResourceLoadState GetAsyncRequestState(const FString& RequestId) const;

    // ��ȡ�첽������ص���Դ��������
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* GetAsyncRequestResource(const FString& RequestId) const;

    // ��ȡ�첽������ص���Դ�������
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<UObject*> GetAsyncRequestResources(const FString& RequestId) const;

    // ȡ���첽����
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void CancelAsyncRequest(const FString& RequestId);

    // ========== ��Դ������� ==========

    // Ԥ�������б��ΪԤ���ص���Դ
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    void PreloadMarkedResources();

    // ����Դ��ӵ�����
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void AddToCache(const FString& ResourcePath, UObject* Resource);

    // �ӻ����ȡ��Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    UObject* GetFromCache(const FString& ResourcePath);

    // �����Դ�Ƿ��ڻ�����
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool IsInCache(const FString& ResourcePath);

    // �ӻ����Ƴ���Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void RemoveFromCache(const FString& ResourcePath);

    // �����Դ����
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void ClearCache();

    // ��ȡ�����С
    UFUNCTION(BlueprintCallable, Category = "Resource")
    int32 GetCacheSize() const;

    // ========== ��Դ��Ϣ��ѯ ==========

    // ��ȡ��Դ��Ϣ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    FResourceInfo GetResourceInfo(const FString& ResourcePath);

    // ��ȡ��Դ��Ϣ
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    bool GetResourceInfoByID(const FName& ResourceID, FResourceTableRow& OutInfo) const;

    // �����Դ�Ƿ����
    UFUNCTION(BlueprintCallable, Category = "Resource")
    bool DoesResourceExist(const FString& ResourcePath);

    // �����ԴID�Ƿ����
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    bool DoesResourceIDExist(const FName& ResourceID) const;

    // ��ȡ�ļ�����������Դ·��
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<FString> GetResourcePathsInFolder(const FString& FolderPath);

    // ��ȡ�ļ�����ָ�����͵�������Դ·��
    UFUNCTION(BlueprintCallable, Category = "Resource")
    TArray<FString> GetResourcePathsInFolderByClass(const FString& FolderPath, TSubclassOf<UObject> ResourceClass);

    // ��ȡ�����µ�������ԴID
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    TArray<FName> GetResourceIDsByCategory(EResourceCategory Category) const;

    // ��ȡ��Դ·��
    UFUNCTION(BlueprintCallable, Category = "Resource|DataTable")
    FString GetResourcePathByID(const FName& ResourceID) const;

    // ========== ��Դж�ع��� ==========

    // ж�ص�����Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void UnloadResource(const FString& ResourcePath);

    // ж���ļ�����������Դ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void UnloadResourcesInFolder(const FString& FolderPath);

    // ǿ����������
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void ForceGarbageCollection();

    // ========== ���Թ��� ==========

    // ��ӡ������Ϣ
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintCacheInfo();

    // ��ӡ�ļ�����Դ�б�
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintFolderResources(const FString& FolderPath);

    // ��ӡ�����첽����״̬
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void PrintAsyncRequests();

private:
    // ��ʽ���ع�����
    FStreamableManager StreamableManager;

    // ��Դ����
    TMap<FString, UObject*> ResourceCache;

    // �첽���ؾ��
    TMap<FString, TSharedPtr<FStreamableHandle>> AsyncHandles;

    // �첽������Ϣ
    TMap<FString, FAsyncLoadRequest> AsyncRequests;

    // ������Դ������
    TMap<FString, UObject*> SingleResourceResults;

    // �����Դ������
    TMap<FString, TArray<UObject*>> MultiResourceResults;

    // �򻯻ص�ӳ��
    TMap<FString, FOnResourceLoadedCallback> SingleResourceCallbacks;
    TMap<FString, FOnResourcesLoadedCallback> MultiResourceCallbacks;

    // ��̬�ص�ӳ��
    TMap<FString, FOnResourceLoadedStaticDelegate> SingleResourceStaticCallbacks;
    TMap<FString, FOnResourcesLoadedStaticDelegate> MultiResourceStaticCallbacks;

    // �ڲ����ط���
    UObject* InternalLoadResourceSync(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass = nullptr);
    TArray<UObject*> InternalLoadResourcesInFolderSync(const FString& FolderPath, TSubclassOf<UObject> ResourceClass = nullptr);

    // ��Դ���ݱ�
    UPROPERTY()
    UDataTable* ResourceDataTable;

    // ��ԴID��·���Ŀ��ٲ��ұ�
    TMap<FName, FString> ResourceIDToPathMap;

    // ���ൽ��ԴID��ӳ��
    TMap<EResourceCategory, TArray<FName>> CategoryToResourceIDsMap;

    // �ڲ��첽���ػص�
    void HandleSingleResourceLoaded(FString RequestId);
    void HandleFolderResourcesLoaded(FString RequestId);

    // �򻯻ص�����
    void HandleSingleResourceWithCallback(FString RequestId);
    void HandleFolderResourcesWithCallback(FString RequestId);

    // ��̬�ص�����
    void HandleSingleResourceWithStaticCallback(FString RequestId);
    void HandleFolderResourcesWithStaticCallback(FString RequestId);

    // �ڲ���̬�ص�����
    void InternalLoadResourceAsyncWithStaticCallback(const FString& ResourcePath, const FOnResourceLoadedStaticDelegate& Callback);
    void InternalLoadResourceAsyncByClassWithStaticCallback(const FString& ResourcePath, TSubclassOf<UObject> ResourceClass, const FOnResourceLoadedStaticDelegate& Callback);
    void InternalLoadResourcesInFolderAsyncWithStaticCallback(const FString& FolderPath, const FOnResourcesLoadedStaticDelegate& Callback);
    void InternalLoadResourcesInFolderAsyncByClassWithStaticCallback(const FString& FolderPath, TSubclassOf<UObject> ResourceClass, const FOnResourcesLoadedStaticDelegate& Callback);

    // ���߷���
    FString SanitizeResourcePath(const FString& ResourcePath) const;
    bool IsValidResourceClass(TSubclassOf<UObject> ResourceClass) const;
    FString GenerateRequestId() const;

    // �ڲ�����
    void BuildLookupTables();
    bool GetResourceTableRow(const FName& ResourceID, FResourceTableRow& OutRow) const;
};