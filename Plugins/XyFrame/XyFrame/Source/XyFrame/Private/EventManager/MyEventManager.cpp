// Fill out your copyright notice in the Description page of Project Settings.

#include "EventManager/MyEventManager.h"
#include "Engine/Engine.h"

// ��̬ʵ������
template<>
UMyEventManager* TSingleton<UMyEventManager>::SingletonInstance = nullptr;

UMyEventManager::UMyEventManager()
{
    // ���캯��
}

UMyEventManager::~UMyEventManager()
{
    // ���������¼�
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

// ========== ͳһ�¼��ӿ�ʵ�� ==========

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

// ========== ��ͼ����C++�¼��ӿ�ʵ�� ==========

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
    // ������ͼ�ɷ���ί��
    OnGameEvent.Broadcast(EventType, EventData);

    // ��¼��־
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

// ========== �¼�����ӿ�ʵ�� ==========

void UMyEventManager::RemoveBlueprintEventByEventname(FName EventName, UObject* object)
{
    if (object)
    {
        // ����ֻ���Ƴ��ض������ָ���������ķ���
        OnGameEvent.Remove(object, EventName);
        UE_LOG(LogTemp, Warning, TEXT("��δ�Ƴ�ָ����������飬���������Ƿ�Ϊָ��������"));
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
    // �Ƴ�������ͼ�¼�
    RemoveAllBlueprintBindings();

    // �Ƴ�����C++�¼�
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
    // ���C++�¼�
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

    // C++�¼�����
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

    // C++�¼�
    UE_LOG(LogTemp, Log, TEXT("C++ Events (%d):"), CppEvents.Num());
    for (const auto& Pair : CppEvents)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: exists"), *Pair.Key.ToString());
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Events ==="));
}

// ========== C++�¼�ʵ�� ==========

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
        // ����򻯴���ʵ��ʹ��ʱ��Ҫ���ݾ���ί�����ͻ�ȡ����
        return 1; // ռλ����ֵ
    }
    return 0;
}