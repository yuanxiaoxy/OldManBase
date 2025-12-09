// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonManager.generated.h"

// 前向声明
class USingletonBase;

/**
 * 单例管理器 - 不继承USingletonBase
 */
UCLASS()
class XYFRAME_API USingletonManager : public UObject
{
    GENERATED_BODY()

public:
    // 获取管理器实例（传统单例模式）
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static USingletonManager* GetInstance();

    // 初始化管理器
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Initialize();

    // 关闭管理器
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    static void Shutdown();

    // 注册单例
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void RegisterSingleton(UObject* Singleton);

    // 注销单例
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void UnregisterSingleton(UClass* SingletonClass);

    // 获取单例
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    UObject* GetSingleton(UClass* SingletonClass) const;

    // 销毁指定类型的单例
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroySingleton(UClass* SingletonClass);

    // 销毁所有单例
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void DestroyAllSingletons();

    // 获取单例数量
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    int32 GetSingletonCount() const;

    // 打印所有单例信息
    UFUNCTION(BlueprintCallable, Category = "SingletonManager")
    void PrintAllSingletons() const;

private:
    // 存储单例实例
    UPROPERTY()
    TMap<UClass*, UObject*> SingletonInstances;

    // 管理器实例
    static USingletonManager* ManagerInstance;
};