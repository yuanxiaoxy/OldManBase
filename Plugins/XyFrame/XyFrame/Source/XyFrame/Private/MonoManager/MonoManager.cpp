// Fill out your copyright notice in the Description page of Project Settings.

#include "MonoManager/MonoManager.h"
#include "Engine/Engine.h"

// 静态实例定义
template<>
UMonoManager* TSingleton<UMonoManager>::SingletonInstance = nullptr;

UMonoManager::UMonoManager()
    : bIsUpdateTimerActive(false)
{
}

UMonoManager::~UMonoManager()
{
    StopUpdateTimer();
    ClearAllTimers();
}

void UMonoManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("MonoManager InitializeSingleton called"));
    InitializeMonoManager();
}

void UMonoManager::InitializeMonoManager()
{
    UE_LOG(LogTemp, Log, TEXT("Mono Manager Initialized"));
}

// ========== 更新计时器系统 ==========

void UMonoManager::StartUpdateTimer()
{
    if (!bIsUpdateTimerActive && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UMonoManager::UpdateTimers, 0.016f, true); // ~60fps
        bIsUpdateTimerActive = true;
        UE_LOG(LogTemp, Log, TEXT("Started MonoManager update timer"));
    }
}

void UMonoManager::StopUpdateTimer()
{
    if (bIsUpdateTimerActive && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
        bIsUpdateTimerActive = false;
        UE_LOG(LogTemp, Log, TEXT("Stopped MonoManager update timer"));
    }
}

void UMonoManager::UpdateTimers()
{
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;

    TArray<FString> TimersToRemove;

    for (auto& TimerPair : Timers)
    {
        FTimerInfo& Timer = TimerPair.Value;

        if (!Timer.bIsActive)
            continue;

        Timer.ElapsedTime += DeltaTime;

        // 执行更新回调
        FTimerUpdateCallbackDelegate* UpdateCallback = UpdateCallbacks.Find(Timer.TimerId);
        if (UpdateCallback && UpdateCallback->IsBound())
        {
            float Progress = GetTimerProgress(Timer.TimerId);
            UpdateCallback->Execute(Timer.TimerId, Progress);
        }

        // 检查计时器是否完成
        if (Timer.ElapsedTime >= Timer.Duration)
        {
            UE_LOG(LogTemp, Log, TEXT("Timer completed: %s"), *Timer.TimerId);
            ExecuteTimerCallback(Timer.TimerId);

            switch (Timer.TimerType)
            {
            case ETimerType::OneShot:
                TimersToRemove.Add(Timer.TimerId);
                break;

            case ETimerType::Interval:
                Timer.ElapsedTime = 0.0f;
                break;

            case ETimerType::Countdown:
                Timer.CurrentLoop++;
                if (Timer.LoopCount > 0 && Timer.CurrentLoop >= Timer.LoopCount)
                {
                    TimersToRemove.Add(Timer.TimerId);
                }
                else
                {
                    Timer.ElapsedTime = 0.0f;
                }
                break;
            }
        }
    }

    for (const FString& TimerId : TimersToRemove)
    {
        ClearTimer(TimerId);
    }

    // 如果没有活动计时器，停止更新计时器
    if (GetActiveTimerCount() == 0)
    {
        StopUpdateTimer();
    }
}

// ========== 内部计时器创建方法 ==========

bool UMonoManager::CreateTimerInternal(const FString& TimerId, ETimerType TimerType, float Duration, int32 LoopCount,
    const FTimerCallbackDelegate& CompleteCallback,
    const FTimerUpdateCallbackDelegate& UpdateCallback,
    const FTimerSimpleDelegate& SimpleCallback)
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

    // 检查是否有有效的回调
    if (!CompleteCallback.IsBound() && !SimpleCallback.IsBound())
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid callback bound for timer"));
        return false;
    }

    // 存储回调
    if (CompleteCallback.IsBound())
    {
        TimerCallbacks.Add(TimerId, CompleteCallback);
    }

    if (SimpleCallback.IsBound())
    {
        SimpleCallbacks.Add(TimerId, SimpleCallback);
    }

    if (UpdateCallback.IsBound())
    {
        UpdateCallbacks.Add(TimerId, UpdateCallback);
    }

    // 创建计时器信息
    FTimerInfo TimerInfo(TimerId, TimerType, Duration);
    TimerInfo.LoopCount = LoopCount;
    Timers.Add(TimerId, TimerInfo);

    // 启动更新计时器
    StartUpdateTimer();

    UE_LOG(LogTemp, Log, TEXT("Created timer: %s, Type: %s, Duration: %.2f, Loops: %d"),
        *TimerId, *UEnum::GetValueAsString(TimerType), Duration, LoopCount);

    return true;
}

// ========== 计时器系统 (自动生成TimerId) ==========

FString UMonoManager::SetTimeout(float Delay, const FTimerCallbackDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetTimeoutSimple(float Delay, const FTimerSimpleDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetInterval(float Interval, const FTimerCallbackDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetIntervalSimple(float Interval, const FTimerSimpleDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetIntervalWithUpdate(float Interval, const FTimerCallbackDelegate& CompleteCallback, const FTimerUpdateCallbackDelegate& UpdateCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, CompleteCallback, UpdateCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetCountdown(float Interval, int32 Count, const FTimerCallbackDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

FString UMonoManager::SetCountdownSimple(float Interval, int32 Count, const FTimerSimpleDelegate& CompleteCallback)
{
    FString TimerId = GenerateTimerId();
    if (CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback))
    {
        return TimerId;
    }
    return FString();
}

// ========== 计时器系统 (自定义TimerId) ==========

bool UMonoManager::SetTimeoutWithId(float Delay, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, CompleteCallback);
}

bool UMonoManager::SetTimeoutSimpleWithId(float Delay, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::OneShot, Delay, 1, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback);
}

bool UMonoManager::SetIntervalWithId(float Interval, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, CompleteCallback);
}

bool UMonoManager::SetIntervalSimpleWithId(float Interval, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::Interval, Interval, 0, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback);
}

bool UMonoManager::SetCountdownWithId(float Interval, int32 Count, const FString& TimerId, const FTimerCallbackDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, CompleteCallback);
}

bool UMonoManager::SetCountdownSimpleWithId(float Interval, int32 Count, const FString& TimerId, const FTimerSimpleDelegate& CompleteCallback)
{
    return CreateTimerInternal(TimerId, ETimerType::Countdown, Interval, Count, FTimerCallbackDelegate(), FTimerUpdateCallbackDelegate(), CompleteCallback);
}

// ========== 计时器控制 ==========

void UMonoManager::PauseTimer(const FString& TimerId)
{
    FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo)
    {
        TimerInfo->bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Paused timer: %s"), *TimerId);
    }
}

void UMonoManager::ResumeTimer(const FString& TimerId)
{
    FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo)
    {
        TimerInfo->bIsActive = true;
        UE_LOG(LogTemp, Log, TEXT("Resumed timer: %s"), *TimerId);
    }
}

void UMonoManager::ClearTimer(const FString& TimerId)
{
    Timers.Remove(TimerId);
    TimerCallbacks.Remove(TimerId);
    SimpleCallbacks.Remove(TimerId);
    UpdateCallbacks.Remove(TimerId);

    UE_LOG(LogTemp, Log, TEXT("Cleared timer: %s"), *TimerId);

    // 如果没有活动计时器，停止更新计时器
    if (GetActiveTimerCount() == 0)
    {
        StopUpdateTimer();
    }
}

void UMonoManager::RestartTimer(const FString& TimerId)
{
    FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo)
    {
        TimerInfo->ElapsedTime = 0.0f;
        TimerInfo->CurrentLoop = 0;
        TimerInfo->bIsActive = true;
        UE_LOG(LogTemp, Log, TEXT("Restarted timer: %s"), *TimerId);
    }
}

void UMonoManager::PauseAllTimers()
{
    for (auto& TimerPair : Timers)
    {
        TimerPair.Value.bIsActive = false;
    }
    UE_LOG(LogTemp, Log, TEXT("Paused all %d timers"), Timers.Num());
}

void UMonoManager::ResumeAllTimers()
{
    for (auto& TimerPair : Timers)
    {
        TimerPair.Value.bIsActive = true;
    }
    UE_LOG(LogTemp, Log, TEXT("Resumed all %d timers"), Timers.Num());
}

void UMonoManager::ClearAllTimers()
{
    int32 Count = Timers.Num();
    Timers.Empty();
    TimerCallbacks.Empty();
    SimpleCallbacks.Empty();
    UpdateCallbacks.Empty();

    StopUpdateTimer();

    UE_LOG(LogTemp, Log, TEXT("Cleared all %d timers"), Count);
}

// ========== 查询方法 ==========

bool UMonoManager::IsTimerActive(const FString& TimerId) const
{
    const FTimerInfo* TimerInfo = Timers.Find(TimerId);
    return TimerInfo && TimerInfo->bIsActive;
}

float UMonoManager::GetTimerRemainingTime(const FString& TimerId) const
{
    const FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo)
    {
        return FMath::Max(0.0f, TimerInfo->Duration - TimerInfo->ElapsedTime);
    }
    return 0.0f;
}

float UMonoManager::GetTimerProgress(const FString& TimerId) const
{
    const FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo && TimerInfo->Duration > 0.0f)
    {
        return FMath::Clamp(TimerInfo->ElapsedTime / TimerInfo->Duration, 0.0f, 1.0f);
    }
    return 0.0f;
}

int32 UMonoManager::GetActiveTimerCount() const
{
    int32 Count = 0;
    for (const auto& TimerPair : Timers)
    {
        if (TimerPair.Value.bIsActive)
        {
            Count++;
        }
    }
    return Count;
}

// ========== 调试工具 ==========

void UMonoManager::PrintAllTimers()
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Timers (%d) ==="), Timers.Num());

    for (const auto& TimerPair : Timers)
    {
        const FTimerInfo& Timer = TimerPair.Value;
        FString TypeString = UEnum::GetValueAsString(Timer.TimerType);
        FString Status = Timer.bIsActive ? TEXT("Active") : TEXT("Paused");
        FString LoopInfo = Timer.LoopCount == 0 ?
            TEXT("Infinite") :
            FString::Printf(TEXT("%d/%d"), Timer.CurrentLoop, Timer.LoopCount);

        UE_LOG(LogTemp, Log, TEXT("  %s: %s [%s] %.2f/%.2f Loops: %s"),
            *Timer.TimerId, *TypeString, *Status, Timer.ElapsedTime, Timer.Duration, *LoopInfo);
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Timers ==="));
}

// ========== 内部实现 ==========

FString UMonoManager::GenerateTimerId() const
{
    return FGuid::NewGuid().ToString();
}

void UMonoManager::ExecuteTimerCallback(const FString& TimerId)
{
    UE_LOG(LogTemp, Log, TEXT("Executing timer callback: %s"), *TimerId);

    // 执行带TimerId的回调
    FTimerCallbackDelegate* TimerCallback = TimerCallbacks.Find(TimerId);
    if (TimerCallback && TimerCallback->IsBound())
    {
        UE_LOG(LogTemp, Log, TEXT("Executing TimerCallback for: %s"), *TimerId);
        TimerCallback->Execute(TimerId);
    }

    // 执行简单无参回调
    FTimerSimpleDelegate* SimpleCallback = SimpleCallbacks.Find(TimerId);
    if (SimpleCallback && SimpleCallback->IsBound())
    {
        UE_LOG(LogTemp, Log, TEXT("Executing SimpleCallback for: %s"), *TimerId);
        SimpleCallback->Execute();
    }

    // 执行成员函数回调 - 使用Lambda
    FTimerInfo* TimerInfo = Timers.Find(TimerId);
    if (TimerInfo && TimerInfo->StaticCallback)
    {
        UE_LOG(LogTemp, Log, TEXT("Executing member function for: %s"), *TimerId);
        TimerInfo->StaticCallback();
    }
}

UWorld* UMonoManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                return Context.World();
            }
        }
    }
    return nullptr;
}