// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "SingletonManager.h"
#include "SingletonBase.generated.h"

/**
 * 单例接口类 - 所有单例类都应该实现这个接口
 */
UINTERFACE(MinimalAPI, Blueprintable)
class USingletonInterface : public UInterface
{
    GENERATED_BODY()
};

class ISingletonInterface
{
    GENERATED_BODY()

public:
    virtual void InitializeSingleton() {}
    virtual void DestroySingleton() {}
};

/**
 * 非Actor单例基类
 */
UCLASS(Abstract)
class XYFRAME_API USingletonBase : public UObject, public ISingletonInterface
{
    GENERATED_BODY()

public:
    virtual ~USingletonBase() override {}

    // ISingletonInterface implementation
    virtual void InitializeSingleton() override {}
    virtual void DestroySingleton() override
    {
        ConditionalBeginDestroy();
    }
};

/**
 * Actor单例基类
 */
UCLASS(Abstract)
class XYFRAME_API ASingletonActor : public AActor, public ISingletonInterface
{
    GENERATED_BODY()

public:
    ASingletonActor()
    {
        // 设置Actor在游戏运行时不被打包
        bReplicates = false;
        SetActorTickEnabled(false);
    }

    // ISingletonInterface implementation
    virtual void InitializeSingleton() override {}
    virtual void DestroySingleton() override
    {
        Destroy();
    }

protected:
    virtual void BeginPlay() override
    {
        Super::BeginPlay();
        InitializeSingleton();
    }

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
    {
        Super::EndPlay(EndPlayReason);
    }
};

/**
 * 增强的单例模板 - 支持Actor和非Actor类型
 */
template <typename T>
class TSingleton
{
public:
    // 获取单例实例
    static T* GetInstance()
    {
        if (!SingletonInstance || !IsValid(SingletonInstance))
        {
            CreateSingletonInstance();
        }
        return SingletonInstance;
    }

    // 销毁单例实例
    static void DestroyInstance()
    {
        if (SingletonInstance && IsValid(SingletonInstance))
        {
            // 调用单例的销毁方法
            if (ISingletonInterface* SingletonInterface = Cast<ISingletonInterface>(SingletonInstance))
            {
                SingletonInterface->DestroySingleton();
            }
            else
            {
                // 如果没有实现接口，使用默认销毁方式
                if constexpr (TIsDerivedFrom<T, AActor>::Value)
                {
                    Cast<AActor>(SingletonInstance)->Destroy();
                }
                else
                {
                    SingletonInstance->ConditionalBeginDestroy();
                }
            }

            // 从管理器注销
            if (USingletonManager* Manager = USingletonManager::GetInstance())
            {
                Manager->UnregisterSingleton(SingletonInstance->GetClass());
            }

            SingletonInstance = nullptr;
            UE_LOG(LogTemp, Log, TEXT("Singleton destroyed: %s"), *T::StaticClass()->GetName());
        }
    }

    // 检查单例是否存在且有效
    static bool IsInstanceValid()
    {
        return SingletonInstance != nullptr && IsValid(SingletonInstance);
    }

    // 设置世界上下文（对于Actor单例）
    static void SetWorldContext(UWorld* World)
    {
        WorldContext = World;
    }

    // 获取当前世界上下文
    static UWorld* GetWorldContext()
    {
        return WorldContext;
    }

protected:
    TSingleton() = default;

private:
    // 创建单例实例
    static void CreateSingletonInstance()
    {
        // 判断类型
        if constexpr (TIsDerivedFrom<T, AActor>::Value)
        {
            SingletonInstance = CreateActorInstance();
        }
        else
        {
            SingletonInstance = CreateUObjectInstance();
        }

        if (SingletonInstance)
        {
            // 注册到管理器
            if (USingletonManager* Manager = USingletonManager::GetInstance())
            {
                Manager->RegisterSingleton(SingletonInstance);
            }

            // 调用初始化
            if (ISingletonInterface* SingletonInterface = Cast<ISingletonInterface>(SingletonInstance))
            {
                SingletonInterface->InitializeSingleton();
            }

            UE_LOG(LogTemp, Log, TEXT("Singleton created: %s"), *SingletonInstance->GetClass()->GetName());
        }
    }

    // 创建UObject实例
    static T* CreateUObjectInstance()
    {
        UObject* Outer = GetTransientPackage();
        T* Instance = NewObject<T>(Outer);
        if (Instance)
        {
            Instance->AddToRoot(); // 防止GC
        }
        return Instance;
    }

    // 创建Actor实例
    static T* CreateActorInstance()
    {
        if (!WorldContext)
        {
            WorldContext = GetGameWorld();
            if (!WorldContext)
            {
                UE_LOG(LogTemp, Error, TEXT("No world context available for actor singleton: %s"), *T::StaticClass()->GetName());
                return nullptr;
            }
        }

        // 查找现有实例
        T* ExistingActor = FindExistingActor();
        if (ExistingActor)
        {
            return ExistingActor;
        }

        // 创建新实例
        return SpawnNewActor();
    }

    // 查找现有的Actor实例
    static T* FindExistingActor()
    {
        TArray<T*> FoundActors;
        for (TActorIterator<T> It(WorldContext); It; ++It)
        {
            T* Actor = *It;
            if (IsValid(Actor))
            {
                FoundActors.Add(Actor);
            }
        }

        if (FoundActors.Num() > 0)
        {
            T* PrimaryActor = FoundActors[0];

            // 销毁重复的实例
            for (int32 i = 1; i < FoundActors.Num(); i++)
            {
                if (FoundActors[i] && IsValid(FoundActors[i]))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Destroying duplicate actor singleton: %s"), *FoundActors[i]->GetName());
                    FoundActors[i]->Destroy();
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Found existing actor singleton: %s"), *PrimaryActor->GetName());
            return PrimaryActor;
        }

        return nullptr;
    }

    // 生成新的Actor实例
    static T* SpawnNewActor()
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.ObjectFlags = RF_Transactional;
        SpawnParams.Name = FName(*FString::Printf(TEXT("%s_Singleton"), *T::StaticClass()->GetName()));

        T* NewActor = WorldContext->SpawnActor<T>(T::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (NewActor)
        {
            UE_LOG(LogTemp, Log, TEXT("Spawned new actor singleton: %s"), *NewActor->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor singleton: %s"), *T::StaticClass()->GetName());
        }

        return NewActor;
    }

    // 获取游戏世界
    static UWorld* GetGameWorld()
    {
        if (GEngine)
        {
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
                {
                    return Context.World();
                }
            }

            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.World())
                {
                    return Context.World();
                }
            }
        }

        return nullptr;
    }

private:
    static T* SingletonInstance;
    static UWorld* WorldContext;
};

// 静态成员定义
template <typename T>
T* TSingleton<T>::SingletonInstance = nullptr;

template <typename T>
UWorld* TSingleton<T>::WorldContext = nullptr;

/**
 * 非Actor单例声明宏
 */
#define DECLARE_SINGLETON(ClassName) \
public: \
    static ClassName* GetInstance() { return TSingleton<ClassName>::GetInstance(); } \
    static void DestroyInstance() { TSingleton<ClassName>::DestroyInstance(); } \
    static bool IsInstanceValid() { return TSingleton<ClassName>::IsInstanceValid(); } \
    static void SetWorldContext(UWorld* World) { TSingleton<ClassName>::SetWorldContext(World); } \
    UFUNCTION(BlueprintCallable, Category = "Singleton") \
    static ClassName* GetSingletonInstance() { return GetInstance(); }

 /**
  * Actor单例声明宏
  */
#define DECLARE_ACTOR_SINGLETON(ClassName) \
    DECLARE_SINGLETON(ClassName) \
    UFUNCTION(BlueprintCallable, Category = "Singleton") \
    static void InitializeActorSingleton(UWorld* World) { SetWorldContext(World); GetInstance(); }