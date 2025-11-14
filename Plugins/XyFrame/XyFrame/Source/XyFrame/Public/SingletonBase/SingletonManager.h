// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "SingletonManager.generated.h"

// 前向声明
class USingletonBase;

/**
 * 单例管理器
 */
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
    static void SetWorldContext(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Shutdown();

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void RegisterSingleton(UObject* Singleton);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void UnregisterSingleton(UClass* SingletonClass);

    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    UObject* GetSingleton(UClass* SingletonClass) const;

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
    static UWorld* CurrentWorldContext;
};