// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Math/Vector.h"
#include "MyEventManager.generated.h"

// 事件数据类型定义
UENUM(BlueprintType)
enum class EGameEventType : uint8
{
    Custom UMETA(DisplayName = "Custom Event"),
    PlayerSpawned UMETA(DisplayName = "Player Spawned"),
    PlayerDied UMETA(DisplayName = "Player Died"),
    LevelCompleted UMETA(DisplayName = "Level Completed"),
    ItemCollected UMETA(DisplayName = "Item Collected"),
    EnemyKilled UMETA(DisplayName = "Enemy Killed"),
};

// 通用事件数据结构
USTRUCT(BlueprintType)
struct FGameEventData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    TArray<FString> Texts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    TArray<float> Values;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    TArray<AActor*> Actors;

    // 默认构造函数
    FGameEventData() {}

    // 便捷构造函数
    FGameEventData(const TArray<FString>& InTexts, const TArray<float>& InValues, const TArray<AActor*>& InActors)
        : Texts(InTexts), Values(InValues), Actors(InActors) {
    }

    // 单字符串构造函数
    FGameEventData(const FString& Text)
    {
        Texts.Add(Text);
    }

    // 单数值构造函数
    FGameEventData(float Value)
    {
        Values.Add(Value);
    }

    // 单Actor构造函数
    FGameEventData(AActor* Actor)
    {
        if (Actor) Actors.Add(Actor);
    }

    // 字符串+数值构造函数
    FGameEventData(const FString& Text, float Value)
    {
        Texts.Add(Text);
        Values.Add(Value);
    }
};

// 通用蓝图事件委托（使用统一的事件数据结构）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameEventSignature, EGameEventType, EventType, const FGameEventData&, EventData);

// C++模板委托声明
template<typename... TArgs>
class TCppEventDelegate;

template<>
class TCppEventDelegate<>
{
public:
    DECLARE_MULTICAST_DELEGATE(FDelegate);
    using FDelegateType = FDelegate;
};

template<typename T1>
class TCppEventDelegate<T1>
{
public:
    DECLARE_MULTICAST_DELEGATE_OneParam(FDelegate, T1);
    using FDelegateType = FDelegate;
};

template<typename T1, typename T2>
class TCppEventDelegate<T1, T2>
{
public:
    DECLARE_MULTICAST_DELEGATE_TwoParams(FDelegate, T1, T2);
    using FDelegateType = FDelegate;
};

template<typename T1, typename T2, typename T3>
class TCppEventDelegate<T1, T2, T3>
{
public:
    DECLARE_MULTICAST_DELEGATE_ThreeParams(FDelegate, T1, T2, T3);
    using FDelegateType = FDelegate;
};

template<typename T1, typename T2, typename T3, typename T4>
class TCppEventDelegate<T1, T2, T3, T4>
{
public:
    DECLARE_MULTICAST_DELEGATE_FourParams(FDelegate, T1, T2, T3, T4);
    using FDelegateType = FDelegate;
};

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UMyEventManager : public USingletonBase
{
    GENERATED_BODY()

    // 声明单例
    DECLARE_SINGLETON(UMyEventManager)

public:
    // 初始化事件管理器
    UFUNCTION(BlueprintCallable, Category = "Event")
    void InitializeEventManager();

    // 重写初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); };

    // 添加一个获取实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Event", meta = (DisplayName = "Get Event Manager"))
    static UMyEventManager* GetEventManager() { return GetInstance(); }

    // 默认构造函数
    UMyEventManager();

    // 析构函数
    virtual ~UMyEventManager() override;

    // ========== 蓝图可分配委托 ==========

    // 全局事件委托 - 蓝图可以直接绑定
    UPROPERTY(BlueprintAssignable, Category = "Event System")
    FOnGameEventSignature OnGameEvent;

    // ========== 统一事件接口（推荐使用） ==========

    // 触发通用事件（蓝图调用）
    UFUNCTION(BlueprintCallable, Category = "Event System")
    void TriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData);

    // 触发通用事件 - 简化版本（蓝图调用）
    UFUNCTION(BlueprintCallable, Category = "Event System")
    void TriggerSimpleGameEvent(EGameEventType EventType, const FString& TextParam = "", float ValueParam = 0.0f, AActor* ActorParam = nullptr);

    // ========== 蓝图调用C++事件接口 ==========

    // 从蓝图触发C++事件 - 无参数版本
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (No Param)"))
    void TriggerCppEvent_NoParam(FName EventName);

    // 从蓝图触发C++事件 - 带事件数据版本
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (With Data)"))
    void TriggerCppEvent_WithData(FName EventName, const FGameEventData& EventData);

    // 从蓝图触发C++事件 - 简单字符串参数
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (String)"))
    void TriggerCppEvent_String(FName EventName, const FString& StringParam);

    // 从蓝图触发C++事件 - 简单整数参数
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (Int)"))
    void TriggerCppEvent_Int(FName EventName, int32 IntParam);

    // 从蓝图触发C++事件 - 简单浮点数参数
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (Float)"))
    void TriggerCppEvent_Float(FName EventName, float FloatParam);

    // ========== 事件管理接口 ==========

    // 移除特定蓝图事件的所有监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveBlueprintEventByEventname(FName EventName, UObject* object);

    // 移除特定对象的所有蓝图事件绑定
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllBlueprintBindingsForObject(UObject* Object);

    // 移除所有事件监听（蓝图和C++）
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllEvents();

    // 移除所有蓝图绑定（保留C++事件）
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllBlueprintBindings();

    // 检查事件是否存在监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasEventListeners(FName EventName) const;

    // 检查特定枚举类型事件是否存在监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasEventListenersByType(EGameEventType EventType) const;

    // 获取事件监听数量
    UFUNCTION(BlueprintCallable, Category = "Event")
    int32 GetEventListenerCount(FName EventName) const;

    // 获取特定枚举类型事件的监听数量
    UFUNCTION(BlueprintCallable, Category = "Event")
    int32 GetEventListenerCountByType(EGameEventType EventType) const;

    // 打印所有事件信息（调试用）
    UFUNCTION(BlueprintCallable, Category = "Event")
    void PrintAllEvents() const;

    // ========== C++事件接口 ==========

    // 注册C++事件监听器（模板方法，支持0-4个任意类型参数）
    template<typename T, typename... TArgs>
    void RegisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...));

    template<typename T, typename... TArgs>
    void RegisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...));

    // 触发C++事件（模板方法，支持0-4个任意类型参数）
    template<typename... TArgs>
    void TriggerCppEventByType(EGameEventType EventType, TArgs... Args);

    template<typename... TArgs>
    void TriggerCppEvent(FName EventName, TArgs... Args);

    // 移除C++事件监听器
    template<typename T, typename... TArgs>
    void UnregisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...));

    template<typename T, typename... TArgs>
    void UnregisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...));

    // 移除特定C++事件的所有监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveCppEvent(FName EventName);

    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveCppEventByType(EGameEventType EventType);

    // 移除所有C++事件监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllCppEvents();

    // 检查C++事件是否存在监听
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasCppEventListeners(FName EventName) const;

    // 获取C++事件监听数量
    int32 GetCppEventListenerCount(FName EventName) const;

private:
    // C++事件字典 - 使用void*存储不同类型的委托
    TMap<FName, TSharedPtr<void>> CppEvents;

    // 内部辅助方法
    template<typename T>
    T* GetCppEventDelegate(FName EventName)
    {
        TSharedPtr<void>* DelegatePtr = CppEvents.Find(EventName);
        if (DelegatePtr && DelegatePtr->IsValid())
        {
            return static_cast<T*>(DelegatePtr->Get());
        }
        return nullptr;
    }

    template<typename T>
    T* GetOrCreateCppEventDelegate(FName EventName)
    {
        TSharedPtr<void>* DelegatePtr = CppEvents.Find(EventName);
        if (DelegatePtr && DelegatePtr->IsValid())
        {
            return static_cast<T*>(DelegatePtr->Get());
        }

        T* NewDelegate = new T();
        CppEvents.Add(EventName, TSharedPtr<void>(NewDelegate));
        return NewDelegate;
    }

    // 内部触发方法
    void InternalTriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData);
};

// 模板函数实现（必须在头文件中）
template<typename T, typename... TArgs>
void UMyEventManager::RegisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...))
{
    using DelegateType = typename TCppEventDelegate<TArgs...>::FDelegateType;
    DelegateType* EventDelegate = GetOrCreateCppEventDelegate<DelegateType>(EventName);
    if (EventDelegate)
    {
        // 使用UE的委托绑定系统
        typename DelegateType::FDelegate Delegate;
        Delegate.BindUObject(Object, Function);
        EventDelegate->Add(Delegate);

        UE_LOG(LogTemp, Log, TEXT("Registered C++ event: %s with %d parameters"),
            *EventName.ToString(), sizeof...(TArgs));
    }
}

template<typename T, typename... TArgs>
void UMyEventManager::RegisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...))
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    RegisterCppEvent<T, TArgs...>(EventName, Object, Function);
}

template<typename... TArgs>
void UMyEventManager::TriggerCppEvent(FName EventName, TArgs... Args)
{
    using DelegateType = typename TCppEventDelegate<TArgs...>::FDelegateType;
    DelegateType* EventDelegate = GetCppEventDelegate<DelegateType>(EventName);
    if (EventDelegate)
    {
        EventDelegate->Broadcast(Args...);
        UE_LOG(LogTemp, Log, TEXT("Triggered C++ event: %s"), *EventName.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("C++ event not found: %s"), *EventName.ToString());
    }
}

template<typename... TArgs>
void UMyEventManager::TriggerCppEventByType(EGameEventType EventType, TArgs... Args)
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    TriggerCppEvent<TArgs...>(EventName, Args);
}

// 取消注册C++事件监听器（对象和成员函数指针版本）
template<typename T, typename... TArgs>
void UMyEventManager::UnregisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...))
{
    using DelegateType = typename TCppEventDelegate<TArgs...>::FDelegateType;
    DelegateType* EventDelegate = GetCppEventDelegate<DelegateType>(EventName);
    if (EventDelegate)
    {
        // 创建一个临时委托用于比较
        typename DelegateType::FDelegate TempDelegate;
        TempDelegate.BindUObject(Object, Function);

        // 移除匹配的委托
        EventDelegate->Remove(TempDelegate);

        // 如果没有监听器了，移除事件
        if (!EventDelegate->IsBound())
        {
            CppEvents.Remove(EventName);
        }

        UE_LOG(LogTemp, Log, TEXT("Unregistered C++ event: %s"), *EventName.ToString());
    }
}

template<typename T, typename... TArgs>
void UMyEventManager::UnregisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...))
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    UnregisterCppEvent<T, TArgs...>(EventName, Object, Function);
}