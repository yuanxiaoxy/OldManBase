// ActorSingletonBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingletonBase/SingletonManager.h"
#include "ActorSingletonBase.generated.h"

UCLASS(Abstract)
class XYFRAME_API AActorSingletonBase : public AActor
{
    GENERATED_BODY()

public:
    AActorSingletonBase();

    UFUNCTION(BlueprintCallable, Category = "Singleton")
    virtual void OnSingletonInit() {}

    UFUNCTION(BlueprintCallable, Category = "Singleton")
    virtual void OnSingletonDestroy() {}

protected:
    virtual void PostInitializeComponents() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

template <typename T>
T* GetInstance(UWorld* World = nullptr)
{
    static_assert(TPointerIsConvertibleFromTo<T, AActorSingletonBase>::Value, "T must derive from AActorSingletonBase");

    USingletonManager* Manager = USingletonManager::GetInstance();
    if (!Manager)
    {
        USingletonManager::Initialize();
        Manager = USingletonManager::GetInstance();
        if (!Manager)
        {
            UE_LOG(LogTemp, Error, TEXT("GetActorSingleton: SingletonManager not available"));
            return nullptr;
        }
    }

    // 1. 精确匹配
    UObject* Existing = Manager->GetSingleton(T::StaticClass());
    T* Instance = Cast<T>(Existing);
    if (Instance && IsValid(Instance))
    {
        return Instance;
    }

    // 2. 向上查找（派生类实例）
    Existing = Manager->GetSingletonDerivedFrom(T::StaticClass());
    Instance = Cast<T>(Existing);
    if (Instance && IsValid(Instance))
    {
        return Instance;
    }

    // 3. 动态生成
    if (!World)
    {
        World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
        if (!World)
        {
            UE_LOG(LogTemp, Error, TEXT("GetActorSingleton: No valid world to spawn actor"));
            return nullptr;
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Instance = World->SpawnActor<T>(SpawnParams);
    return Instance;
}

template <typename T>
bool IsActorSingletonValid()
{
    static_assert(TPointerIsConvertibleFromTo<T, AActorSingletonBase>::Value, "T must derive from AActorSingletonBase");
    USingletonManager* Manager = USingletonManager::GetInstance();
    if (!Manager) return false;
    UObject* Existing = Manager->GetSingletonDerivedFrom(T::StaticClass());
    return Existing != nullptr && IsValid(Existing);
}

template <typename T>
void DestroyActorSingleton()
{
    static_assert(TPointerIsConvertibleFromTo<T, AActorSingletonBase>::Value, "T must derive from AActorSingletonBase");
    USingletonManager::GetInstance()->DestroySingleton(T::StaticClass());
}

#define DECLARE_ACTOR_SINGLETON(ClassName) \
public: \
    static ClassName* GetInstance() { return ::GetInstance<ClassName>(); } \
    static void DestroyInstance() { ::DestroyActorSingleton<ClassName>(); } \
    static bool IsInstanceValid() { return ::IsActorSingletonValid<ClassName>(); }