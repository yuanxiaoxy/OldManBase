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
 * 单例基类 - 所有单例类都应该继承这个类
 */
UCLASS(Abstract)
class XYFRAME_API USingletonBase : public UObject
{
    GENERATED_BODY()

public:
    virtual ~USingletonBase() {}

    // 虚函数，用于子类重写以进行初始化
    virtual void InitializeSingleton() {}

    // 虚函数，用于子类重写以进行销毁
    UFUNCTION()
    virtual void DestroyCurSingleton() {}
};

/**
 * 增强的单例模板 - 自动注册到管理器，支持Actor和非Actor类型
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
            // 判断是否是Actor类型
            if constexpr (TIsDerivedFrom<T, AActor>::Value)
            {
                // Actor类型：查找场景中已有的实例或生成新的
                SingletonInstance = FindOrSpawnActorInstance();
            }
            else
            {
                // 非Actor类型：创建UObject实例
                SingletonInstance = NewObject<T>();
                if (SingletonInstance)
                {
                    SingletonInstance->AddToRoot(); // 防止GC
                }
            }

            if (SingletonInstance)
            {
                // 自动注册到单例管理器
                if (USingletonManager* Manager = USingletonManager::GetInstance())
                {
                    Manager->RegisterSingleton(SingletonInstance);
                }

                // 调用初始化方法
                SingletonInstance->InitializeSingleton();

                UE_LOG(LogTemp, Log, TEXT("Singleton created and registered: %s"), *SingletonInstance->GetClass()->GetName());
            }
        }
        return SingletonInstance;
    }

    // 销毁单例实例
    static void DestroyInstance()
    {
        if (SingletonInstance && IsValid(SingletonInstance))
        {
            // 从管理器注销
            if (USingletonManager* Manager = USingletonManager::GetInstance())
            {
                Manager->UnregisterSingleton(SingletonInstance->GetClass());
            }

            if constexpr (TIsDerivedFrom<T, AActor>::Value)
            {
                // Actor类型：调用Destroy
                AActor* ActorInstance = Cast<AActor>(SingletonInstance);
                if (ActorInstance)
                {
                    ActorInstance->Destroy();
                }
            }
            else
            {
                // 非Actor类型：移除Root并销毁
                SingletonInstance->RemoveFromRoot();
                SingletonInstance->ConditionalBeginDestroy();
            }

            SingletonInstance = nullptr;

            UE_LOG(LogTemp, Log, TEXT("Singleton destroyed and unregistered"));
        }
    }

    // 检查单例是否存在
    static bool IsInstanceValid()
    {
        return SingletonInstance != nullptr && IsValid(SingletonInstance);
    }

    // 设置世界上下文（对于Actor单例，在获取实例前需要设置）
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
    // 查找或生成Actor实例（仅对Actor类型有效）
    static T* FindOrSpawnActorInstance()
    {
        if (!WorldContext)
        {
            // 尝试自动获取世界上下文
            WorldContext = GetGameWorld();
            if (!WorldContext)
            {
                UE_LOG(LogTemp, Error, TEXT("Cannot get world context for actor singleton"));
                return nullptr;
            }
        }

        TArray<T*> FoundActors;
        for (TActorIterator<T> It(WorldContext); It; ++It)
        {
            T* Actor = *It;
            if (IsValid(Actor))
            {
                FoundActors.Add(Actor);
            }
        }

        T* ResultActor = nullptr;

        if (FoundActors.Num() > 0)
        {
            // 使用第一个找到的Actor
            ResultActor = FoundActors[0];

            // 销毁其他多余的实例
            for (int32 i = 1; i < FoundActors.Num(); i++)
            {
                if (FoundActors[i] && IsValid(FoundActors[i]))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Destroying duplicate actor singleton: %s"), *FoundActors[i]->GetName());
                    FoundActors[i]->Destroy();
                }
            }

            UE_LOG(LogTemp, Log, TEXT("Found existing actor singleton: %s"), *ResultActor->GetName());
        }
        else
        {
            // 没有找到，生成新的Actor实例
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            SpawnParams.ObjectFlags = RF_Transactional;

            ResultActor = WorldContext->SpawnActor<T>(SpawnParams);
            if (ResultActor)
            {
                UE_LOG(LogTemp, Log, TEXT("Spawned new actor singleton: %s"), *ResultActor->GetName());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to spawn actor singleton: %s"), *T::StaticClass()->GetName());
            }
        }

        return ResultActor;
    }

    // 获取游戏世界
    static UWorld* GetGameWorld()
    {
        // 尝试从GEngine获取
        if (GEngine)
        {
            // 优先获取PIE世界
            for (const FWorldContext& Context : GEngine->GetWorldContexts())
            {
                if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
                {
                    return Context.World();
                }
            }

            // 如果没有PIE/Game世界，使用第一个有效的世界
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
 * 单例声明宏 - 包含蓝图可调用方法
 */
#define DECLARE_SINGLETON(ClassName) \
public: \
    /* C++静态方法 */ \
    static ClassName* GetInstance() { return TSingleton<ClassName>::GetInstance(); } \
    static void DestroyInstance() { TSingleton<ClassName>::DestroyInstance(); } \
    static bool IsInstanceValid() { return TSingleton<ClassName>::IsInstanceValid(); } \
    static void SetWorldContext(UWorld* World) { TSingleton<ClassName>::SetWorldContext(World); }

 /**
  * Actor单例专用宏 - 提供额外的世界上下文设置
  */
#define DECLARE_ACTOR_SINGLETON(ClassName) \
    DECLARE_SINGLETON(ClassName) \
    UFUNCTION(BlueprintCallable, Category = "Singleton") \
    static void InitializeActorSingleton(UWorld* World) { SetWorldContext(World); GetInstance(); }