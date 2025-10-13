// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "MonoManager.generated.h"

// ��ʱ���ص�ί��
DECLARE_DYNAMIC_DELEGATE_OneParam(FTimerCallbackDelegate, const FString&, TimerId);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTimerUpdateCallbackDelegate, const FString&, TimerId, float, Progress);
DECLARE_DYNAMIC_DELEGATE(FTimerSimpleDelegate);

// ��ʱ������
UENUM(BlueprintType)
enum class ETimerType : uint8
{
    OneShot UMETA(DisplayName = "One Shot"),
    Interval UMETA(DisplayName = "Interval"),
    Countdown UMETA(DisplayName = "Countdown")
};

// ��ʱ����Ϣ
USTRUCT(BlueprintType)
struct FTimerInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    FString TimerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    ETimerType TimerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    int32 LoopCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    int32 CurrentLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
    bool bIsActive;

    // �洢�ص���Ϣ - ʹ��Lambda�����Ǻ�����
    TWeakObjectPtr<UObject> CallbackObject;
    TFunction<void()> StaticCallback;

    FTimerInfo()
        : TimerType(ETimerType::OneShot)
        , Duration(0.0f)
        , ElapsedTime(0.0f)
        , LoopCount(1)
        , CurrentLoop(0)
        , bIsActive(false)
    {
    }

    FTimerInfo(const FString& InTimerId, ETimerType InTimerType, float InDuration)
        : TimerId(InTimerId)
        , TimerType(InTimerType)
        , Duration(InDuration)
        , ElapsedTime(0.0f)
        , LoopCount(1)
        , CurrentLoop(0)
        , bIsActive(true)
    {
        if (InTimerType == ETimerType::Interval)
        {
            LoopCount = 0;
        }
    }
};

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UMonoManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UMonoManager)

public:
    UFUNCTION(BlueprintCallable, Category = "MonoManager")
    void InitializeMonoManager();

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); };

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MonoManager", meta = (DisplayName = "Get Mono Manager"))
    static UMonoManager* GetMonoManager() { return GetInstance(); }

    UMonoManager();
    virtual ~UMonoManager() override;

    // ========== ��ʱ��ϵͳ (�Զ�����TimerId) ==========
    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer", meta = (DisplayName = "Set Timeout"))
    FString SetTimeout(float Delay, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer", meta = (DisplayName = "Set Timeout Simple"))
    FString SetTimeoutSimple(float Delay, const FTimerSimpleDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer", meta = (DisplayName = "Set Interval"))
    FString SetInterval(float Interval, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer", meta = (DisplayName = "Set Interval Simple"))
    FString SetIntervalSimple(float Interval, const FTimerSimpleDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    FString SetIntervalWithUpdate(float Interval, const FTimerCallbackDelegate& CompleteCallback, const FTimerUpdateCallbackDelegate& UpdateCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    FString SetCountdown(float Interval, int32 Count, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    FString SetCountdownSimple(float Interval, int32 Count, const FTimerSimpleDelegate& CompleteCallback);

    // ========== ��ʱ��ϵͳ (�Զ���TimerId) ==========
    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetTimeoutWithId(float Delay, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetTimeoutSimpleWithId(float Delay, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetIntervalWithId(float Interval, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetIntervalSimpleWithId(float Interval, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetCountdownWithId(float Interval, int32 Count, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool SetCountdownSimpleWithId(float Interval, int32 Count, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback);

    // ========== C++��Ա������ ==========
// �󶨳�Ա���� - һ���Զ�ʱ�� (�Զ�����TimerId)
    template<typename T>
    FString SetTimeout(float Delay, T* Object, void(T::* Function)())
    {
        FString TimerId = GenerateTimerId();
        if (CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, Object, Function))
        {
            return TimerId;
        }
        return FString();
    }

    // �󶨳�Ա���� - һ���Զ�ʱ�� (�Զ���TimerId)
    template<typename T>
    bool SetTimeout(float Delay, const FString& TimerId, T* Object, void(T::* Function)())
    {
        return CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, Object, Function);
    }

    // �󶨳�Ա���� - �����ʱ�� (�Զ�����TimerId)
    template<typename T>
    FString SetInterval(float Interval, T* Object, void(T::* Function)())
    {
        FString TimerId = GenerateTimerId();
        if (CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, Object, Function))
        {
            return TimerId;
        }
        return FString();
    }

    // �󶨳�Ա���� - �����ʱ�� (�Զ���TimerId)
    template<typename T>
    bool SetInterval(float Interval, const FString& TimerId, T* Object, void(T::* Function)())
    {
        return CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, Object, Function);
    }

    // �󶨳�Ա���� - ����ʱ�� (�Զ�����TimerId)
    template<typename T>
    FString SetCountdown(float Interval, int32 Count, T* Object, void(T::* Function)())
    {
        FString TimerId = GenerateTimerId();
        if (CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, Object, Function))
        {
            return TimerId;
        }
        return FString();
    }

    // �󶨳�Ա���� - ����ʱ�� (�Զ���TimerId)
    template<typename T>
    bool SetCountdown(float Interval, int32 Count, const FString& TimerId, T* Object, void(T::* Function)())
    {
        return CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, Object, Function);
    }

    // ========== ��ʱ������ ==========
    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void PauseTimer(const FString& TimerId);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void ResumeTimer(const FString& TimerId);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void ClearTimer(const FString& TimerId);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void RestartTimer(const FString& TimerId);

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void PauseAllTimers();

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void ResumeAllTimers();

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    void ClearAllTimers();

    // ========== ��ѯ���� ==========
    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    bool IsTimerActive(const FString& TimerId) const;

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    float GetTimerRemainingTime(const FString& TimerId) const;

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    float GetTimerProgress(const FString& TimerId) const;

    UFUNCTION(BlueprintCallable, Category = "MonoManager|Timer")
    int32 GetActiveTimerCount() const;

    // ========== ���Թ��� ==========
    UFUNCTION(BlueprintCallable, Category = "MonoManager|Debug")
    void PrintAllTimers();

protected:
    // ʹ��UE��Timerϵͳ����������
    void StartUpdateTimer();
    void StopUpdateTimer();
    UFUNCTION()
    void UpdateTimers();

private:
    TMap<FString, FTimerInfo> Timers;

    // �ص��洢
    TMap<FString, FTimerCallbackDelegate> TimerCallbacks;
    TMap<FString, FTimerSimpleDelegate> SimpleCallbacks;
    TMap<FString, FTimerUpdateCallbackDelegate> UpdateCallbacks;

    // UE Timer handle
    FTimerHandle UpdateTimerHandle;
    bool bIsUpdateTimerActive;

    FString GenerateTimerId() const;
    void ExecuteTimerCallback(const FString& TimerId);

    bool CreateTimerInternal(const FString& TimerId, ETimerType TimerType, float Duration, int32 LoopCount,
        const FTimerCallbackDelegate& CompleteCallback = FTimerCallbackDelegate(),
        const FTimerUpdateCallbackDelegate& UpdateCallback = FTimerUpdateCallbackDelegate(),
        const FTimerSimpleDelegate& SimpleCallback = FTimerSimpleDelegate());

    // ��Ա�����󶨰汾 - ʹ�ú������ַ���
    template<typename T>
    bool CreateTimerInternal(const FString& TimerId, ETimerType TimerType, float Duration, int32 LoopCount, T* Object, void(T::* Function)())
    {
        if (Duration <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot create timer with duration <= 0"));
            return false;
        }

        if (Timers.Contains(TimerId))
        {
            UE_LOG(LogTemp, Warning, TEXT("TimerId already exists: %s"), *TimerId);
            return false;
        }

        if (!Object || !Function)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid object or function for timer"));
            return false;
        }

        // ������ʱ����Ϣ
        FTimerInfo TimerInfo(TimerId, TimerType, Duration);
        TimerInfo.LoopCount = LoopCount;
        TimerInfo.CallbackObject = Object;

        // ʹ��Lambda�������Ա����ָ��
        TimerInfo.StaticCallback = [Object, Function]() {
            if (Object && Function)
            {
                (Object->*Function)();
            }
            };

        Timers.Add(TimerId, TimerInfo);

        // �������¼�ʱ���������Ҫ��
        StartUpdateTimer();

        UE_LOG(LogTemp, Log, TEXT("Created timer with member function: %s, Type: %s, Duration: %.2f, Loops: %d"),
            *TimerId, *UEnum::GetValueAsString(TimerType), Duration, LoopCount);

        return true;
    }

    UWorld* GetWorld() const override;
};