// ActorSingletonBase.cpp
#include "SingletonBase/ActorSingletonBase.h"

AActorSingletonBase::AActorSingletonBase()
{
    // 可在此设置默认属性，如 PrimaryActorTick.bCanEverTick = false;
}

void AActorSingletonBase::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    USingletonManager* Manager = USingletonManager::GetInstance();
    if (!Manager)
    {
        USingletonManager::Initialize();
        Manager = USingletonManager::GetInstance();
        if (!Manager)
        {
            UE_LOG(LogTemp, Error, TEXT("AActorSingletonBase: Failed to initialize SingletonManager"));
            return;
        }
    }

    UClass* MyClass = GetClass();

    // 检查是否有已注册实例与当前类存在继承关系
    bool bHasConflict = false;
    UObject* ConflictInstance = nullptr;
    for (const auto& Pair : Manager->GetAllSingletons())
    {
        UObject* Inst = Pair.Value;
        if (Inst && IsValid(Inst) && Inst != this)
        {
            UClass* InstClass = Inst->GetClass();
            if (MyClass->IsChildOf(InstClass) || InstClass->IsChildOf(MyClass))
            {
                bHasConflict = true;
                ConflictInstance = Inst;
                break;
            }
        }
    }

    if (bHasConflict)
    {
        UE_LOG(LogTemp, Warning, TEXT("ActorSingleton %s: conflict with existing instance of %s, destroying self."),
            *MyClass->GetName(), *ConflictInstance->GetClass()->GetName());
        Destroy();
        return;
    }

    // 无冲突，若尚未注册则注册自身
    if (!Manager->GetSingleton(MyClass))
    {
        Manager->RegisterSingleton(this);
        OnSingletonInit();
        UE_LOG(LogTemp, Log, TEXT("ActorSingleton %s registered."), *MyClass->GetName());
    }
}

void AActorSingletonBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    USingletonManager* Manager = USingletonManager::GetInstance();
    if (Manager)
    {
        UClass* MyClass = GetClass();
        UObject* Current = Manager->GetSingleton(MyClass);
        if (Current == this)
        {
            OnSingletonDestroy();
            Manager->UnregisterSingleton(MyClass);
            UE_LOG(LogTemp, Log, TEXT("ActorSingleton %s unregistered."), *MyClass->GetName());
        }
    }
    Super::EndPlay(EndPlayReason);
}