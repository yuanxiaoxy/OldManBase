// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ObjectPoolManager.generated.h"

// ����ؽӿ� - ʹ��UE�Ľӿ�ϵͳ
UINTERFACE(Blueprintable)
class UObjectPoolInterface : public UInterface
{
    GENERATED_BODY()
};

class IObjectPoolInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
    void OnSpawn();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
    void OnDespawn();
};

// �������Ϣ
USTRUCT(BlueprintType)
struct FObjectPoolInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    FString PoolName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    int32 Capacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    int32 PreloadCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    int32 UsedCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
    int32 UnusedCount;

    FObjectPoolInfo()
        : Capacity(-1)
        , PreloadCount(0)
        , UsedCount(0)
        , UnusedCount(0)
    {
    }
};

// ���������
UCLASS()
class XYFRAME_API UObjectPool : public UObject
{
    GENERATED_BODY()

public:
    // ��ʼ�������
    void Initialize(TSubclassOf<AActor> InActorClass, int32 InCapacity = -1);

    // �Ӷ�������ɶ���
    AActor* Spawn(UWorld* World, const FVector& Location, const FRotator& Rotation,
        AActor* Owner = nullptr, APawn* Instigator = nullptr);

    // ���ն��󵽶����
    void Despawn(AActor* Actor, float DelayTime = 0.0f);

    // �������ն����ڲ�ʹ�ã�
    void DespawnImmediate(AActor* Actor);

    // ������������ʹ�õĶ���
    void DespawnAll();

    // Ԥ���ض���
    void Preload(UWorld* World, int32 Amount = 1);

    // ��ȡ�������Ϣ
    FObjectPoolInfo GetPoolInfo() const;

    // ��������
    void SetCapacity(int32 NewCapacity);

    // ��������
    void ClearPool();

private:
    // ʵ��ִ�л��յĶ�ʱ���ص�
    UFUNCTION()
    void ExecuteDespawn(AActor* Actor);

private:
    UPROPERTY()
    TSubclassOf<AActor> ActorClass;

    int32 Capacity;

    UPROPERTY()
    TArray<AActor*> UsedActors;

    UPROPERTY()
    TArray<AActor*> UnusedActors;

    friend class UObjectPoolManager;
};

// ����ع�����
UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UObjectPoolManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UObjectPoolManager)

public:
    // ��ʼ������ع�����
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void InitializeObjectPoolManager();

    // ��д������ʼ������
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // ��ȡ������ʵ������ͼ�ɵ��÷���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectPool", meta = (DisplayName = "Get ObjectPool Manager"))
    static UObjectPoolManager* GetObjectPoolManager() { return GetInstance(); }

    // ���캯��
    UObjectPoolManager();
    virtual ~UObjectPoolManager() override;

    // ========== ��Ҫ�ӿ� ==========

    // �Ӷ�������ɶ���
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    AActor* Spawn(TSubclassOf<AActor> ActorClass, const FVector& Location, const FRotator& Rotation,
        AActor* Owner = nullptr, APawn* Instigator = nullptr);

    // ���ն��󵽶���أ�֧���ӳ٣�
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void Despawn(AActor* Actor, float DelayTime = 0.0f);

    // ����ָ�����͵����ж���
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void DespawnAllByClass(TSubclassOf<AActor> ActorClass);

    // �������ж���
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void DespawnAll();

    // Ԥ���ض���
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void Preload(TSubclassOf<AActor> ActorClass, int32 Amount = 1);

    // ���ö��������
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void SetCapacity(TSubclassOf<AActor> ActorClass, int32 Capacity = -1);

    // ��ȡ���������
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    int32 GetCapacity(TSubclassOf<AActor> ActorClass);

    // ========== ��ѯ�͵��� ==========

    // ��ȡ�������Ϣ
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    FObjectPoolInfo GetPoolInfo(TSubclassOf<AActor> ActorClass);

    // ��ȡ���ж������Ϣ
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    TArray<FObjectPoolInfo> GetAllPoolInfos();

    // ��ӡ���ж������Ϣ
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void PrintAllPools();

    // �������Ƿ��ɶ���ع���
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    bool IsManagedByPool(AActor* Actor);

    // ��ȡ����Ķ�������
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    int32 GetTotalManagedCount() const;

private:
    // ���һ򴴽������
    UObjectPool* FindOrCreatePool(TSubclassOf<AActor> ActorClass);

    // ���Ҷ����
    UObjectPool* FindPool(TSubclassOf<AActor> ActorClass);

    // ͨ��������Ҷ����
    UObjectPool* FindPoolByActor(AActor* Actor);

    // ���󵽶���ص�ӳ��
    UPROPERTY()
    TMap<TSubclassOf<AActor>, UObjectPool*> PoolsMap;

    // ���󵽶���صĿ��ٲ���
    UPROPERTY()
    TMap<AActor*, UObjectPool*> ActorToPoolMap;

    // ����ظ�����������֯��
    UPROPERTY()
    UObject* PoolsParent;

    UWorld* GetWorld() const override;
};