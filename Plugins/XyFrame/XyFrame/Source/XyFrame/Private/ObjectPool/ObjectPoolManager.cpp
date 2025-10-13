// Fill out your copyright notice in the Description page of Project Settings.

#include "ObjectPool/ObjectPoolManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

// ��̬ʵ������
template<>
UObjectPoolManager* TSingleton<UObjectPoolManager>::SingletonInstance = nullptr;

// ========== UObjectPool ʵ�� ==========

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

    // ��δʹ���б��л�ȡ����
    if (UnusedActors.Num() > 0)
    {
        Actor = UnusedActors[0];
        UnusedActors.RemoveAt(0);

        // ����λ�ú���ת
        Actor->SetActorLocationAndRotation(Location, Rotation);

        // ����Owner��Instigator
        Actor->SetOwner(Owner);
        if (APawn* PawnActor = Cast<APawn>(Actor))
        {
            PawnActor->SetInstigator(Instigator);
        }

        // �������
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
        Actor->SetActorTickEnabled(true);
    }
    else
    {
        // �����¶���
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Owner;
        SpawnParams.Instigator = Instigator;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        Actor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);
    }

    if (Actor)
    {
        UsedActors.Add(Actor);

        // ����OnSpawn�ص� - �޸��ӿڵ��÷�ʽ
        if (Actor->GetClass()->ImplementsInterface(UObjectPoolInterface::StaticClass()))
        {
            IObjectPoolInterface::Execute_OnSpawn(Actor);
        }

        // ���Ե�����ͼ�¼�
        UFunction* OnSpawnFunc = Actor->FindFunction(FName("OnSpawn"));
        if (OnSpawnFunc)
        {
            Actor->ProcessEvent(OnSpawnFunc, nullptr);
        }

        // ���Ե�����ͼʵ�ֵĽӿڷ���
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
        // ʹ�ö�ʱ���ӳٻ���
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
        // ��������
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

    // �����������
    if (Capacity >= 0 && UnusedActors.Num() >= Capacity)
    {
        // �����������������ϵĶ���
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

    // ͣ�ö���
    Actor->SetActorHiddenInGame(true);
    Actor->SetActorEnableCollision(false);
    Actor->SetActorTickEnabled(false);

    // ����λ�õ�Զ�볡���ĵط�
    Actor->SetActorLocation(FVector(0, 0, -10000));

    // �ƶ���δʹ���б�
    UnusedActors.Add(Actor);
    UsedActors.Remove(Actor);

    // ����OnDespawn�ص� - �޸��ӿڵ��÷�ʽ
    if (Actor->GetClass()->ImplementsInterface(UObjectPoolInterface::StaticClass()))
    {
        IObjectPoolInterface::Execute_OnDespawn(Actor);
    }

    // ���Ե�����ͼ�¼�
    UFunction* OnDespawnFunc = Actor->FindFunction(FName("OnDespawn"));
    if (OnDespawnFunc)
    {
        Actor->ProcessEvent(OnDespawnFunc, nullptr);
    }

    // ���Ե�����ͼʵ�ֵĽӿڷ���
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
            // ����ͣ�ò�����δʹ���б�
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
    Info.PreloadCount = UnusedActors.Num(); // Ԥ�����������ڵ�ǰδʹ������
    Info.PoolName = ActorClass ? ActorClass->GetName() : TEXT("Invalid");

    return Info;
}

void UObjectPool::SetCapacity(int32 NewCapacity)
{
    Capacity = NewCapacity;

    // ���������С�ڵ�ǰδʹ�ö�����������Ҫ���ٶ������
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
    // �������ж���
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

// ========== UObjectPoolManager ʵ�� ==========

UObjectPoolManager::UObjectPoolManager()
{
    PoolsParent = nullptr;
}

UObjectPoolManager::~UObjectPoolManager()
{
    // �������ж����
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

    // ��������ظ�����
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
        // ��¼���󵽳ص�ӳ��
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
        // ע�⣺����������ӳ�����Ƴ�����Ϊ������ܻ���ʹ����
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

    // �����¶����
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