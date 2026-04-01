// SingletonManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonManager.generated.h"

class USingletonBase;

UCLASS()
class XYFRAME_API USingletonManager : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static USingletonManager* GetInstance();

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Initialize();

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Shutdown();

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void RegisterSingleton(UObject* Singleton);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void UnregisterSingleton(UClass* SingletonClass);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    UObject* GetSingleton(UClass* SingletonClass) const;

    // 获取第一个派生自 BaseClass 的单例实例（用于基类查找派生类）
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    UObject* GetSingletonDerivedFrom(UClass* BaseClass) const;

    // 获取所有单例的只读映射（用于冲突检测）
    const TMap<UClass*, UObject*>& GetAllSingletons() const { return SingletonInstances; }

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroySingleton(UClass* SingletonClass);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroyAllSingletons();

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    int32 GetSingletonCount() const;

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void PrintAllSingletons() const;

private:
    UPROPERTY()
    TMap<UClass*, UObject*> SingletonInstances;

    static USingletonManager* ManagerInstance;
};