// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ObjectPoolManager.generated.h"

// 对象池接口 - 使用UE的接口系统
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

// 对象池信息
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

// 单个对象池
UCLASS()
class XYFRAME_API UObjectPool : public UObject
{
    GENERATED_BODY()

public:
    // 初始化对象池
    void Initialize(TSubclassOf<AActor> InActorClass, int32 InCapacity = -1);

    // 从对象池生成对象
    AActor* Spawn(UWorld* World, const FVector& Location, const FRotator& Rotation,
        AActor* Owner = nullptr, APawn* Instigator = nullptr);

    // 回收对象到对象池
    void Despawn(AActor* Actor, float DelayTime = 0.0f);

    // 立即回收对象（内部使用）
    void DespawnImmediate(AActor* Actor);

    // 回收所有正在使用的对象
    void DespawnAll();

    // 预加载对象
    void Preload(UWorld* World, int32 Amount = 1);

    // 获取对象池信息
    FObjectPoolInfo GetPoolInfo() const;

    // 设置容量
    void SetCapacity(int32 NewCapacity);

    // 清理对象池
    void ClearPool();

private:
    // 实际执行回收的定时器回调
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

// 对象池管理器
UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UObjectPoolManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UObjectPoolManager)

public:
    // 初始化对象池管理器
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void InitializeObjectPoolManager();

    // 重写单例初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // 获取管理器实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectPool", meta = (DisplayName = "Get ObjectPool Manager"))
    static UObjectPoolManager* GetObjectPoolManager() { return GetInstance(); }

    // 构造函数
    UObjectPoolManager();
    virtual ~UObjectPoolManager() override;

    // ========== 主要接口 ==========

    // 从对象池生成对象
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    AActor* Spawn(TSubclassOf<AActor> ActorClass, const FVector& Location, const FRotator& Rotation,
        AActor* Owner = nullptr, APawn* Instigator = nullptr);

    // 回收对象到对象池（支持延迟）
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void Despawn(AActor* Actor, float DelayTime = 0.0f);

    // 回收指定类型的所有对象
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void DespawnAllByClass(TSubclassOf<AActor> ActorClass);

    // 回收所有对象
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void DespawnAll();

    // 预加载对象
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void Preload(TSubclassOf<AActor> ActorClass, int32 Amount = 1);

    // 设置对象池容量
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void SetCapacity(TSubclassOf<AActor> ActorClass, int32 Capacity = -1);

    // 获取对象池容量
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    int32 GetCapacity(TSubclassOf<AActor> ActorClass);

    // ========== 查询和调试 ==========

    // 获取对象池信息
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    FObjectPoolInfo GetPoolInfo(TSubclassOf<AActor> ActorClass);

    // 获取所有对象池信息
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    TArray<FObjectPoolInfo> GetAllPoolInfos();

    // 打印所有对象池信息
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    void PrintAllPools();

    // 检查对象是否由对象池管理
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    bool IsManagedByPool(AActor* Actor);

    // 获取管理的对象总数
    UFUNCTION(BlueprintCallable, Category = "ObjectPool")
    int32 GetTotalManagedCount() const;

private:
    // 查找或创建对象池
    UObjectPool* FindOrCreatePool(TSubclassOf<AActor> ActorClass);

    // 查找对象池
    UObjectPool* FindPool(TSubclassOf<AActor> ActorClass);

    // 通过对象查找对象池
    UObjectPool* FindPoolByActor(AActor* Actor);

    // 对象到对象池的映射
    UPROPERTY()
    TMap<TSubclassOf<AActor>, UObjectPool*> PoolsMap;

    // 对象到对象池的快速查找
    UPROPERTY()
    TMap<AActor*, UObjectPool*> ActorToPoolMap;

    // 对象池父对象（用于组织）
    UPROPERTY()
    UObject* PoolsParent;

    UWorld* GetWorld() const override;
};