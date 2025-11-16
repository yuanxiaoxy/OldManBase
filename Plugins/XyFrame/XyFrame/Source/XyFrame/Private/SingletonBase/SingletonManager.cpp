// Fill out your copyright notice in the Description page of Project Settings.

#include "SingletonBase/SingletonManager.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/Engine.h"

USingletonManager* USingletonManager::ManagerInstance = nullptr;
UWorld* USingletonManager::CurrentWorldContext = nullptr;

USingletonManager* USingletonManager::GetInstance()
{
    if (!ManagerInstance)
    {
        ManagerInstance = NewObject<USingletonManager>();
        ManagerInstance->AddToRoot();
        UE_LOG(LogTemp, Log, TEXT("SingletonManager created"));
    }
    return ManagerInstance;
}

void USingletonManager::Initialize()
{
    GetInstance(); // 确保实例存在
    UE_LOG(LogTemp, Log, TEXT("SingletonManager initialized"));
}

void USingletonManager::SetWorldContext(UWorld* World)
{
    CurrentWorldContext = World;
    UE_LOG(LogTemp, Log, TEXT("SingletonManager world context set: %s"),
        World ? *World->GetName() : TEXT("None"));
}

void USingletonManager::Shutdown()
{
    if (ManagerInstance)
    {
        ManagerInstance->DestroyAllSingletons();
        ManagerInstance->RemoveFromRoot();
        ManagerInstance->ConditionalBeginDestroy();
        ManagerInstance = nullptr;
        CurrentWorldContext = nullptr;
        UE_LOG(LogTemp, Log, TEXT("SingletonManager shutdown"));
    }
}

void USingletonManager::RegisterSingleton(UObject* Singleton)
{
    if (Singleton && IsValid(Singleton))
    {
        UClass* ClassType = Singleton->GetClass();
        if (!SingletonInstances.Contains(ClassType))
        {
            SingletonInstances.Add(ClassType, Singleton);
            UE_LOG(LogTemp, Log, TEXT("Registered singleton: %s"), *ClassType->GetName());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Singleton already registered: %s"), *ClassType->GetName());
        }
    }
}

void USingletonManager::UnregisterSingleton(UClass* SingletonClass)
{
    if (SingletonInstances.Contains(SingletonClass))
    {
        SingletonInstances.Remove(SingletonClass);
        UE_LOG(LogTemp, Log, TEXT("Unregistered singleton: %s"), *SingletonClass->GetName());
    }
}

UObject* USingletonManager::GetSingleton(UClass* SingletonClass) const
{
    UObject* const* SingletonPtr = SingletonInstances.Find(SingletonClass);
    return SingletonPtr ? *SingletonPtr : nullptr;
}

void USingletonManager::DestroySingleton(UClass* SingletonClass)
{
    UObject* Singleton = GetSingleton(SingletonClass);
    if (Singleton && IsValid(Singleton))
    {
        // 使用接口进行销毁
        if (ISingletonInterface* SingletonInterface = Cast<ISingletonInterface>(Singleton))
        {
            SingletonInterface->DestroySingleton();
        }
        else
        {
            // 回退到类型特定的销毁方式
            if (AActor* ActorSingleton = Cast<AActor>(Singleton))
            {
                ActorSingleton->Destroy();
            }
            else
            {
                Singleton->ConditionalBeginDestroy();
            }
        }

        UnregisterSingleton(SingletonClass);
    }
}

void USingletonManager::DestroyAllSingletons()
{
    UE_LOG(LogTemp, Log, TEXT("Destroying all %d singletons"), SingletonInstances.Num());

    // 创建临时数组，避免在迭代过程中修改容器
    TArray<UClass*> SingletonClasses;
    SingletonInstances.GetKeys(SingletonClasses);

    for (UClass* SingletonClass : SingletonClasses)
    {
        DestroySingleton(SingletonClass);
    }

    SingletonInstances.Empty();
}

int32 USingletonManager::GetSingletonCount() const
{
    return SingletonInstances.Num();
}

void USingletonManager::PrintAllSingletons() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Registered Singletons (%d) ==="), SingletonInstances.Num());
    for (const auto& Pair : SingletonInstances)
    {
        FString Status = (Pair.Value && IsValid(Pair.Value)) ? "Valid" : "Invalid";
        FString Type = Pair.Value->IsA<AActor>() ? "Actor" : "UObject";
        UE_LOG(LogTemp, Log, TEXT("  %s: %s (%s)"), *Pair.Key->GetName(), *Status, *Type);
    }
    UE_LOG(LogTemp, Log, TEXT("=== End Singletons ==="));
}