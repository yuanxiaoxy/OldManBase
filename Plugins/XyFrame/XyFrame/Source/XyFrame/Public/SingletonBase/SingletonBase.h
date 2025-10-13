// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonManager.h"  // 直接包含头文件
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
 * 增强的单例模板 - 自动注册到管理器
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
            // 创建单例实例
            SingletonInstance = NewObject<T>();
            if (SingletonInstance)
            {
                SingletonInstance->AddToRoot(); // 防止GC

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

            SingletonInstance->RemoveFromRoot();
            SingletonInstance->ConditionalBeginDestroy();
            SingletonInstance = nullptr;

            UE_LOG(LogTemp, Log, TEXT("Singleton destroyed and unregistered"));
        }
    }

    // 检查单例是否存在
    static bool IsInstanceValid()
    {
        return SingletonInstance != nullptr && IsValid(SingletonInstance);
    }

protected:
    TSingleton() = default;

private:
    static T* SingletonInstance;
};

// 静态成员定义
template <typename T>
T* TSingleton<T>::SingletonInstance = nullptr;

/**
 * 单例声明宏 - 包含蓝图可调用方法
 */
#define DECLARE_SINGLETON(ClassName) \
public: \
    /* C++静态方法 */ \
    static ClassName* GetInstance() { return TSingleton<ClassName>::GetInstance(); } \
    static void DestroyInstance() { TSingleton<ClassName>::DestroyInstance(); } \
    static bool IsInstanceValid() { return TSingleton<ClassName>::IsInstanceValid(); } 