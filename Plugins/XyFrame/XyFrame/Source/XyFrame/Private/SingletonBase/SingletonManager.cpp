// SingletonManager.cpp
#include "SingletonBase/SingletonManager.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/Engine.h"

USingletonManager* USingletonManager::ManagerInstance = nullptr;

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
    GetInstance();
    UE_LOG(LogTemp, Log, TEXT("SingletonManager initialized"));
}

void USingletonManager::Shutdown()
{
    if (ManagerInstance)
    {
        ManagerInstance->DestroyAllSingletons();
        ManagerInstance->RemoveFromRoot();
        ManagerInstance->ConditionalBeginDestroy();
        ManagerInstance = nullptr;
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

UObject* USingletonManager::GetSingletonDerivedFrom(UClass* BaseClass) const
{
    for (const auto& Pair : SingletonInstances)
    {
        UObject* Instance = Pair.Value;
        if (Instance && IsValid(Instance) && Instance->IsA(BaseClass))
        {
            return Instance;
        }
    }
    return nullptr;
}

void USingletonManager::DestroySingleton(UClass* SingletonClass)
{
    USingletonBase* Singleton = Cast<USingletonBase>(GetSingleton(SingletonClass));
    if (Singleton && IsValid(Singleton))
    {
        UFunction* DestroyFunc = Singleton->FindFunction(FName("DestroyCurSingleton"));
        if (DestroyFunc)
        {
            Singleton->ProcessEvent(DestroyFunc, nullptr);
        }
        else
        {
            Singleton->RemoveFromRoot();
            Singleton->ConditionalBeginDestroy();
            UnregisterSingleton(SingletonClass);
        }
    }
}

void USingletonManager::DestroyAllSingletons()
{
    UE_LOG(LogTemp, Log, TEXT("Destroying all %d singletons"), SingletonInstances.Num());
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
        UE_LOG(LogTemp, Log, TEXT("  %s: %s"), *Pair.Key->GetName(), *Status);
    }
    UE_LOG(LogTemp, Log, TEXT("=== End Singletons ==="));
}