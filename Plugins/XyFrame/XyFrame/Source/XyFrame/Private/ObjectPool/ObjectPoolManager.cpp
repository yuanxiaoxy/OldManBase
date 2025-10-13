// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPool/ObjectPoolManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// 静态实例定义
template<>
UObjectPoolManager* TSingleton<UObjectPoolManager>::SingletonInstance = nullptr;

// ========== UObjectPool 实现 ==========

void UObjectPool::Initialize(TSubclassOf<AActor> InActorClass, int32 InCapacity)
{
    ActorClass = InActorClass;
    Capacity = InCapacity;
    UsedActors.Empty();
    UnusedActors.Empty();
}

AActor* UObjectPool::Spawn(UWorld* World, const FVector& Location, const FRotator& Rotation,
    AActor* Owner, APawn* Instigator)
{
    if (!World || !ActorClass)
    {
        return nullptr;
    }

    AActor* Actor = nullptr;

    // 从未使用列表中获取对象
    if (UnusedActors.Num() > 0)
    {
        Actor = UnusedActors[0];
        UnusedActors.RemoveAt(0);

        // 设置位置和旋转
        Actor->SetActorLocationAndRotation(Location, Rotation);

        // 设置Owner和Instigator
        Actor->SetOwner(Owner);
        if (APawn* PawnActor = Cast<APawn>(Actor))
        {
            PawnActor->SetInstigator(Instigator);
        }

        // 激活对象
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
        Actor->SetActorTickEnabled(true);
    }
    else
    {
        // 创建新对象
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Owner;
        SpawnParams.Instigator = Instigator;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        Actor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
    }

    if (Actor)
    {
        UsedActors.Add(Actor);

        // 调用OnSpawn回调 - 修复接口调用方式
        if (Actor->GetClass()->ImplementsInterface(UObjectPoolInterface::StaticClass()))
        {
            IObjectPoolInterface::Execute_OnSpawn(Actor);
        }

        // 尝试调用蓝图事件
        UFunction* OnSpawnFunc = Actor->FindFunction(FName("OnSpawn"));
        if (OnSpawnFunc)
        {
            Actor->ProcessEvent(OnSpawnFunc, nullptr);
        }

        // 尝试调用蓝图实现的接口方法
        UFunction* OnSpawnInterfaceFunc = Actor->FindFunction(FName("Execute_OnSpawn"));
        if (OnSpawnInterfaceFunc)
        {
            Actor->ProcessEvent(OnSpawnInterfaceFunc, nullptr);
        }
    }

    return Actor;
}

void UObjectPool::Despawn(AActor* Actor, float DelayTime)
{
    if (!Actor)
    {
        return;
    }

    if (DelayTime > 0.0f)
    {
        // 使用定时器延迟回收
        if (UWorld* World = Actor->GetWorld())
        {
            FTimerHandle TimerHandle;
            FTimerDelegate TimerDelegate;
            TimerDelegate.BindUObject(this, &UObjectPool::ExecuteDespawn, Actor);
            World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, DelayTime, false);
        }
    }
    else
    {
        // 立即回收
        DespawnImmediate(Actor);
    }
}

void UObjectPool::ExecuteDespawn(AActor* Actor)
{
    DespawnImmediate(Actor);
}

void UObjectPool::DespawnImmediate(AActor* Actor)
{
    if (!Actor || !UsedActors.Contains(Actor))
    {
        return;
    }

    // 检查容量限制
    if (Capacity >= 0 && UnusedActors.Num() >= Capacity)
    {
        // 超过容量，销毁最老的对象
        if (UnusedActors.Num() > 0)
        {
            AActor* OldActor = UnusedActors[0];
            UnusedActors.RemoveAt(0);
            if (OldActor)
            {
                OldActor->Destroy();
            }
        }
    }

    // 停用对象
    Actor->SetActorHiddenInGame(true);
    Actor->SetActorEnableCollision(false);
    Actor->SetActorTickEnabled(false);

    // 重置位置到远离场景的地方
    Actor->SetActorLocation(FVector(0, 0, -10000));

    // 移动到未使用列表
    UnusedActors.Add(Actor);
    UsedActors.Remove(Actor);

    // 调用OnDespawn回调 - 修复接口调用方式
    if (Actor->GetClass()->ImplementsInterface(UObjectPoolInterface::StaticClass()))
    {
        IObjectPoolInterface::Execute_OnDespawn(Actor);
    }

    // 尝试调用蓝图事件
    UFunction* OnDespawnFunc = Actor->FindFunction(FName("OnDespawn"));
    if (OnDespawnFunc)
    {
        Actor->ProcessEvent(OnDespawnFunc, nullptr);
    }

    // 尝试调用蓝图实现的接口方法
    UFunction* OnDespawnInterfaceFunc = Actor->FindFunction(FName("Execute_OnDespawn"));
    if (OnDespawnInterfaceFunc)
    {
        Actor->ProcessEvent(OnDespawnInterfaceFunc, nullptr);
    }
}

void UObjectPool::DespawnAll()
{
    TArray<AActor*> ActorsToDespawn = UsedActors;
    for (AActor* Actor : ActorsToDespawn)
    {
        DespawnImmediate(Actor);
    }
}

void UObjectPool::Preload(UWorld* World, int32 Amount)
{
    if (!World || !ActorClass || Amount <= 0)
    {
        return;
    }

    for (int32 i = 0; i < Amount; i++)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* Actor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (Actor)
        {
            // 立即停用并放入未使用列表
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorEnableCollision(false);
            Actor->SetActorTickEnabled(false);
            Actor->SetActorLocation(FVector(0, 0, -10000));

            UnusedActors.Add(Actor);
        }
    }
}

FObjectPoolInfo UObjectPool::GetPoolInfo() const
{
    FObjectPoolInfo Info;
    Info.ActorClass = ActorClass;
    Info.Capacity = Capacity;
    Info.UsedCount = UsedActors.Num();
    Info.UnusedCount = UnusedActors.Num();
    Info.PreloadCount = UnusedActors.Num(); // 预加载数量等于当前未使用数量
    Info.PoolName = ActorClass ? ActorClass->GetName() : TEXT("Invalid");

    return Info;
}

void UObjectPool::SetCapacity(int32 NewCapacity)
{
    Capacity = NewCapacity;

    // 如果新容量小于当前未使用对象数量，需要销毁多余对象
    if (Capacity >= 0 && UnusedActors.Num() > Capacity)
    {
        int32 ExcessCount = UnusedActors.Num() - Capacity;
        for (int32 i = 0; i < ExcessCount; i++)
        {
            if (UnusedActors.Num() > 0)
            {
                AActor* Actor = UnusedActors[0];
                UnusedActors.RemoveAt(0);
                if (Actor)
                {
                    Actor->Destroy();
                }
            }
        }
    }
}

void UObjectPool::ClearPool()
{
    // 销毁所有对象
    for (AActor* Actor : UsedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }

    for (AActor* Actor : UnusedActors)
    {
        if (Actor)
        {
            Actor->Destroy();
        }
    }

    UsedActors.Empty();
    UnusedActors.Empty();
}

// ========== UObjectPoolManager 实现 ==========

UObjectPoolManager::UObjectPoolManager()
{
    PoolsParent = nullptr;
}

UObjectPoolManager::~UObjectPoolManager()
{
    // 清理所有对象池
    for (auto& PoolPair : PoolsMap)
    {
        if (PoolPair.Value)
        {
            PoolPair.Value->ClearPool();
        }
    }
    PoolsMap.Empty();
    ActorToPoolMap.Empty();
}

void UObjectPoolManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("ObjectPoolManager InitializeSingleton called"));
    InitializeObjectPoolManager();
}

void UObjectPoolManager::InitializeObjectPoolManager()
{
    UE_LOG(LogTemp, Log, TEXT("ObjectPool Manager Initialized"));

    // 创建对象池父对象
    PoolsParent = NewObject<UObject>(this);
    PoolsParent->SetFlags(RF_Standalone);
}

AActor* UObjectPoolManager::Spawn(TSubclassOf<AActor> ActorClass, const FVector& Location,
    const FRotator& Rotation, AActor* Owner, APawn* Instigator)
{
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn actor with null class"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn actor without valid world"));
        return nullptr;
    }

    UObjectPool* Pool = FindOrCreatePool(ActorClass);
    if (!Pool)
    {
        return nullptr;
    }

    AActor* Actor = Pool->Spawn(World, Location, Rotation, Owner, Instigator);
    if (Actor)
    {
        // 记录对象到池的映射
        ActorToPoolMap.Add(Actor, Pool);
    }

    return Actor;
}

void UObjectPoolManager::Despawn(AActor* Actor, float DelayTime)
{
    if (!Actor)
    {
        return;
    }

    UObjectPool* Pool = FindPoolByActor(Actor);
    if (Pool)
    {
        Pool->Despawn(Actor, DelayTime);
        // 注意：不在立即从映射中移除，因为对象可能还在使用中
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor not managed by object pool: %s"), *Actor->GetName());
    }
}

void UObjectPoolManager::DespawnAllByClass(TSubclassOf<AActor> ActorClass)
{
    UObjectPool* Pool = FindPool(ActorClass);
    if (Pool)
    {
        Pool->DespawnAll();
    }
}

void UObjectPoolManager::DespawnAll()
{
    for (auto& PoolPair : PoolsMap)
    {
        if (PoolPair.Value)
        {
            PoolPair.Value->DespawnAll();
        }
    }
    ActorToPoolMap.Empty();
}

void UObjectPoolManager::Preload(TSubclassOf<AActor> ActorClass, int32 Amount)
{
    if (!ActorClass || Amount <= 0)
    {
        return;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return;
    }

    UObjectPool* Pool = FindOrCreatePool(ActorClass);
    if (Pool)
    {
        Pool->Preload(World, Amount);
    }
}

void UObjectPoolManager::SetCapacity(TSubclassOf<AActor> ActorClass, int32 Capacity)
{
    UObjectPool* Pool = FindOrCreatePool(ActorClass);
    if (Pool)
    {
        Pool->SetCapacity(Capacity);
    }
}

int32 UObjectPoolManager::GetCapacity(TSubclassOf<AActor> ActorClass)
{
    UObjectPool* Pool = FindPool(ActorClass);
    if (Pool)
    {
        return Pool->Capacity;
    }
    return -1;
}

FObjectPoolInfo UObjectPoolManager::GetPoolInfo(TSubclassOf<AActor> ActorClass)
{
    UObjectPool* Pool = FindPool(ActorClass);
    if (Pool)
    {
        return Pool->GetPoolInfo();
    }

    return FObjectPoolInfo();
}

TArray<FObjectPoolInfo> UObjectPoolManager::GetAllPoolInfos()
{
    TArray<FObjectPoolInfo> Infos;
    for (auto& PoolPair : PoolsMap)
    {
        if (PoolPair.Value)
        {
            Infos.Add(PoolPair.Value->GetPoolInfo());
        }
    }
    return Infos;
}

void UObjectPoolManager::PrintAllPools()
{
    UE_LOG(LogTemp, Log, TEXT("=== Object Pools Info ==="));

    TArray<FObjectPoolInfo> Infos = GetAllPoolInfos();
    for (const FObjectPoolInfo& Info : Infos)
    {
        UE_LOG(LogTemp, Log, TEXT("Pool: %s, Class: %s, Used: %d, Unused: %d, Capacity: %d"),
            *Info.PoolName,
            Info.ActorClass ? *Info.ActorClass->GetName() : TEXT("None"),
            Info.UsedCount,
            Info.UnusedCount,
            Info.Capacity);
    }

    UE_LOG(LogTemp, Log, TEXT("Total Managed Actors: %d"), GetTotalManagedCount());
    UE_LOG(LogTemp, Log, TEXT("=== End Object Pools Info ==="));
}

bool UObjectPoolManager::IsManagedByPool(AActor* Actor)
{
    return ActorToPoolMap.Contains(Actor);
}

int32 UObjectPoolManager::GetTotalManagedCount() const
{
    int32 Total = 0;
    for (auto& PoolPair : PoolsMap)
    {
        if (PoolPair.Value)
        {
            Total += PoolPair.Value->UsedActors.Num() + PoolPair.Value->UnusedActors.Num();
        }
    }
    return Total;
}

UObjectPool* UObjectPoolManager::FindOrCreatePool(TSubclassOf<AActor> ActorClass)
{
    UObjectPool** PoolPtr = PoolsMap.Find(ActorClass);
    if (PoolPtr && *PoolPtr)
    {
        return *PoolPtr;
    }

    // 创建新对象池
    UObjectPool* NewPool = NewObject<UObjectPool>(PoolsParent);
    NewPool->Initialize(ActorClass);
    PoolsMap.Add(ActorClass, NewPool);

    UE_LOG(LogTemp, Log, TEXT("Created new object pool for class: %s"), *ActorClass->GetName());

    return NewPool;
}

UObjectPool* UObjectPoolManager::FindPool(TSubclassOf<AActor> ActorClass)
{
    UObjectPool** PoolPtr = PoolsMap.Find(ActorClass);
    return PoolPtr ? *PoolPtr : nullptr;
}

UObjectPool* UObjectPoolManager::FindPoolByActor(AActor* Actor)
{
    UObjectPool** PoolPtr = ActorToPoolMap.Find(Actor);
    return PoolPtr ? *PoolPtr : nullptr;
}

UWorld* UObjectPoolManager::GetWorld() const
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