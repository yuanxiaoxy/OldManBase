// Fill out your copyright notice in the Description page of Project Settings.

#include "EventManager/MyEventManager.h"
#include "Engine/Engine.h"

// 静态实例定义
template<>
UMyEventManager* TSingleton<UMyEventManager>::SingletonInstance = nullptr;

UMyEventManager::UMyEventManager()
{
    // 构造函数
}

UMyEventManager::~UMyEventManager()
{
    // 清理所有事件
    RemoveAllEvents();
    RemoveAllCppEvents();
}

void UMyEventManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("EventManager InitializeSingleton called"));
    InitializeEventManager();
}

void UMyEventManager::InitializeEventManager()
{
    UE_LOG(LogTemp, Log, TEXT("Event Manager Initialized"));
}

// ========== 统一事件接口实现 ==========

void UMyEventManager::TriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    InternalTriggerGameEvent(EventType, EventData);
}

void UMyEventManager::TriggerSimpleGameEvent(EGameEventType EventType, const FString& TextParam, float ValueParam, AActor* ActorParam)
{
    FGameEventData EventData;

    if (!TextParam.IsEmpty())
    {
        EventData.Texts.Add(TextParam);
    }

    if (ValueParam != 0.0f)
    {
        EventData.Values.Add(ValueParam);
    }

    if (ActorParam != nullptr)
    {
        EventData.Actors.Add(ActorParam);
    }

    InternalTriggerGameEvent(EventType, EventData);
}

// ========== 蓝图调用C++事件接口实现 ==========

void UMyEventManager::TriggerCppEvent_NoParam(FName EventName)
{
    TriggerCppEvent<>(EventName);
}

void UMyEventManager::TriggerCppEvent_WithData(FName EventName, const FGameEventData& EventData)
{
    TriggerCppEvent<FGameEventData>(EventName, EventData);
}

void UMyEventManager::TriggerCppEvent_String(FName EventName, const FString& StringParam)
{
    TriggerCppEvent<FString>(EventName, StringParam);
}

void UMyEventManager::TriggerCppEvent_Int(FName EventName, int32 IntParam)
{
    TriggerCppEvent<int32>(EventName, IntParam);
}

void UMyEventManager::TriggerCppEvent_Float(FName EventName, float FloatParam)
{
    TriggerCppEvent<float>(EventName, FloatParam);
}

void UMyEventManager::InternalTriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    // 触发蓝图可分配委托
    OnGameEvent.Broadcast(EventType, EventData);

    // 记录日志
    FString LogMessage = FString::Printf(TEXT("Triggered Game Event: %s"), *UEnum::GetValueAsString(EventType));

    if (EventData.Texts.Num() > 0)
    {
        LogMessage += FString::Printf(TEXT(", Texts: %d"), EventData.Texts.Num());
    }

    if (EventData.Values.Num() > 0)
    {
        LogMessage += FString::Printf(TEXT(", Values: %d"), EventData.Values.Num());
    }

    if (EventData.Actors.Num() > 0)
    {
        LogMessage += FString::Printf(TEXT(", Actors: %d"), EventData.Actors.Num());
    }

    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

// ========== 事件管理接口实现 ==========

void UMyEventManager::RemoveBlueprintEventByEventname(FName EventName, UObject* object)
{
    if (object)
    {
        // 我们只能移除特定对象的指定方法名的方法
        OnGameEvent.Remove(object, EventName);
        UE_LOG(LogTemp, Warning, TEXT("若未移除指定方法，检查，给定名字是否为指定方法名"));
    }
}

void UMyEventManager::RemoveAllBlueprintBindingsForObject(UObject* Object)
{
    if (Object)
    {
        OnGameEvent.RemoveAll(Object);
        UE_LOG(LogTemp, Log, TEXT("Removed all blueprint bindings for object: %s"), *Object->GetName());
    }
}

void UMyEventManager::RemoveAllEvents()
{
    // 移除所有蓝图事件
    RemoveAllBlueprintBindings();

    // 移除所有C++事件
    RemoveAllCppEvents();

    UE_LOG(LogTemp, Log, TEXT("Removed all events"));
}

void UMyEventManager::RemoveAllBlueprintBindings()
{
    OnGameEvent.RemoveAll(this);
    UE_LOG(LogTemp, Log, TEXT("Removed all blueprint bindings"));
}

bool UMyEventManager::HasEventListeners(FName EventName) const
{
    // 检查C++事件
    bool bHasCppListeners = HasCppEventListeners(EventName);

    return bHasCppListeners;
}

bool UMyEventManager::HasEventListenersByType(EGameEventType EventType) const
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    return HasEventListeners(EventName);
}

int32 UMyEventManager::GetEventListenerCount(FName EventName) const
{
    int32 Count = 0;

    // C++事件数量
    Count += GetCppEventListenerCount(EventName);

    return Count;
}

int32 UMyEventManager::GetEventListenerCountByType(EGameEventType EventType) const
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    return GetEventListenerCount(EventName);
}

void UMyEventManager::PrintAllEvents() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Registered Events ==="));

    // C++事件
    UE_LOG(LogTemp, Log, TEXT("C++ Events (%d):"), CppEvents.Num());
    for (const auto& Pair : CppEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: exists"), *Pair.Key.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Events ==="));
}

// ========== C++事件实现 ==========

void UMyEventManager::RemoveCppEvent(FName EventName)
{
    CppEvents.Remove(EventName);
}

void UMyEventManager::RemoveCppEventByType(EGameEventType EventType)
{
    FName EventName = FName(*UEnum::GetValueAsString(EventType));
    RemoveCppEvent(EventName);
}

void UMyEventManager::RemoveAllCppEvents()
{
    CppEvents.Empty();
}

bool UMyEventManager::HasCppEventListeners(FName EventName) const
{
    return CppEvents.Contains(EventName);
}

int32 UMyEventManager::GetCppEventListenerCount(FName EventName) const
{
    const TSharedPtr<void>* DelegatePtr = CppEvents.Find(EventName);
    if (DelegatePtr && DelegatePtr->IsValid())
    {
        // 这里简化处理，实际使用时需要根据具体委托类型获取数量
        return 1; // 占位返回值
    }
    return 0;
}