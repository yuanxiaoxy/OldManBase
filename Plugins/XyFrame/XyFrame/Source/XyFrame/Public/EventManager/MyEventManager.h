// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Math/Vector.h"
#include "MyEventManager.generated.h"

// �¼��������Ͷ���
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

// ͨ���¼����ݽṹ
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

    // Ĭ�Ϲ��캯��
    FGameEventData() {}

    // ��ݹ��캯��
    FGameEventData(const TArray<FString>& InTexts, const TArray<float>& InValues, const TArray<AActor*>& InActors)
        : Texts(InTexts), Values(InValues), Actors(InActors) {
    }

    // ���ַ������캯��
    FGameEventData(const FString& Text)
    {
        Texts.Add(Text);
    }

    // ����ֵ���캯��
    FGameEventData(float Value)
    {
        Values.Add(Value);
    }

    // ��Actor���캯��
    FGameEventData(AActor* Actor)
    {
        if (Actor) Actors.Add(Actor);
    }

    // �ַ���+��ֵ���캯��
    FGameEventData(const FString& Text, float Value)
    {
        Texts.Add(Text);
        Values.Add(Value);
    }
};

// ͨ����ͼ�¼�ί�У�ʹ��ͳһ���¼����ݽṹ��
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameEventSignature, EGameEventType, EventType, const FGameEventData&, EventData);

// C++ģ��ί������
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

    // ��������
    DECLARE_SINGLETON(UMyEventManager)

public:
    // ��ʼ���¼�������
    UFUNCTION(BlueprintCallable, Category = "Event")
    void InitializeEventManager();

    // ��д��ʼ������
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); };

    // ���һ����ȡʵ������ͼ�ɵ��÷���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Event", meta = (DisplayName = "Get Event Manager"))
    static UMyEventManager* GetEventManager() { return GetInstance(); }

    // Ĭ�Ϲ��캯��
    UMyEventManager();

    // ��������
    virtual ~UMyEventManager() override;

    // ========== ��ͼ�ɷ���ί�� ==========

    // ȫ���¼�ί�� - ��ͼ����ֱ�Ӱ�
    UPROPERTY(BlueprintAssignable, Category = "Event System")
    FOnGameEventSignature OnGameEvent;

    // ========== ͳһ�¼��ӿڣ��Ƽ�ʹ�ã� ==========

    // ����ͨ���¼�����ͼ���ã�
    UFUNCTION(BlueprintCallable, Category = "Event System")
    void TriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData);

    // ����ͨ���¼� - �򻯰汾����ͼ���ã�
    UFUNCTION(BlueprintCallable, Category = "Event System")
    void TriggerSimpleGameEvent(EGameEventType EventType, const FString& TextParam = "", float ValueParam = 0.0f, AActor* ActorParam = nullptr);

    // ========== ��ͼ����C++�¼��ӿ� ==========

    // ����ͼ����C++�¼� - �޲����汾
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (No Param)"))
    void TriggerCppEvent_NoParam(FName EventName);

    // ����ͼ����C++�¼� - ���¼����ݰ汾
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (With Data)"))
    void TriggerCppEvent_WithData(FName EventName, const FGameEventData& EventData);

    // ����ͼ����C++�¼� - ���ַ�������
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (String)"))
    void TriggerCppEvent_String(FName EventName, const FString& StringParam);

    // ����ͼ����C++�¼� - ����������
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (Int)"))
    void TriggerCppEvent_Int(FName EventName, int32 IntParam);

    // ����ͼ����C++�¼� - �򵥸���������
    UFUNCTION(BlueprintCallable, Category = "Event System", meta = (DisplayName = "Trigger C++ Event (Float)"))
    void TriggerCppEvent_Float(FName EventName, float FloatParam);

    // ========== �¼�����ӿ� ==========

    // �Ƴ��ض���ͼ�¼������м���
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveBlueprintEventByEventname(FName EventName, UObject* object);

    // �Ƴ��ض������������ͼ�¼���
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllBlueprintBindingsForObject(UObject* Object);

    // �Ƴ������¼���������ͼ��C++��
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllEvents();

    // �Ƴ�������ͼ�󶨣�����C++�¼���
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllBlueprintBindings();

    // ����¼��Ƿ���ڼ���
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasEventListeners(FName EventName) const;

    // ����ض�ö�������¼��Ƿ���ڼ���
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasEventListenersByType(EGameEventType EventType) const;

    // ��ȡ�¼���������
    UFUNCTION(BlueprintCallable, Category = "Event")
    int32 GetEventListenerCount(FName EventName) const;

    // ��ȡ�ض�ö�������¼��ļ�������
    UFUNCTION(BlueprintCallable, Category = "Event")
    int32 GetEventListenerCountByType(EGameEventType EventType) const;

    // ��ӡ�����¼���Ϣ�������ã�
    UFUNCTION(BlueprintCallable, Category = "Event")
    void PrintAllEvents() const;

    // ========== C++�¼��ӿ� ==========

    // ע��C++�¼���������ģ�巽����֧��0-4���������Ͳ�����
    template<typename T, typename... TArgs>
    void RegisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...));

    template<typename T, typename... TArgs>
    void RegisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...));

    // ����C++�¼���ģ�巽����֧��0-4���������Ͳ�����
    template<typename... TArgs>
    void TriggerCppEventByType(EGameEventType EventType, TArgs... Args);

    template<typename... TArgs>
    void TriggerCppEvent(FName EventName, TArgs... Args);

    // �Ƴ�C++�¼�������
    template<typename T, typename... TArgs>
    void UnregisterCppEventByType(EGameEventType EventType, T* Object, void (T::* Function)(TArgs...));

    template<typename T, typename... TArgs>
    void UnregisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...));

    // �Ƴ��ض�C++�¼������м���
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveCppEvent(FName EventName);

    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveCppEventByType(EGameEventType EventType);

    // �Ƴ�����C++�¼�����
    UFUNCTION(BlueprintCallable, Category = "Event")
    void RemoveAllCppEvents();

    // ���C++�¼��Ƿ���ڼ���
    UFUNCTION(BlueprintCallable, Category = "Event")
    bool HasCppEventListeners(FName EventName) const;

    // ��ȡC++�¼���������
    int32 GetCppEventListenerCount(FName EventName) const;

private:
    // C++�¼��ֵ� - ʹ��void*�洢��ͬ���͵�ί��
    TMap<FName, TSharedPtr<void>> CppEvents;

    // �ڲ���������
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

    // �ڲ���������
    void InternalTriggerGameEvent(EGameEventType EventType, const FGameEventData& EventData);
};

// ģ�庯��ʵ�֣�������ͷ�ļ��У�
template<typename T, typename... TArgs>
void UMyEventManager::RegisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...))
{
    using DelegateType = typename TCppEventDelegate<TArgs...>::FDelegateType;
    DelegateType* EventDelegate = GetOrCreateCppEventDelegate<DelegateType>(EventName);
    if (EventDelegate)
    {
        // ʹ��UE��ί�а�ϵͳ
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

// ȡ��ע��C++�¼�������������ͳ�Ա����ָ��汾��
template<typename T, typename... TArgs>
void UMyEventManager::UnregisterCppEvent(FName EventName, T* Object, void (T::* Function)(TArgs...))
{
    using DelegateType = typename TCppEventDelegate<TArgs...>::FDelegateType;
    DelegateType* EventDelegate = GetCppEventDelegate<DelegateType>(EventName);
    if (EventDelegate)
    {
        // ����һ����ʱί�����ڱȽ�
        typename DelegateType::FDelegate TempDelegate;
        TempDelegate.BindUObject(Object, Function);

        // �Ƴ�ƥ���ί��
        EventDelegate->Remove(TempDelegate);

        // ���û�м������ˣ��Ƴ��¼�
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