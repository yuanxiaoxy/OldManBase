// Fill out your copyright notice in the Description page of Project Settings.

#include "TaskSystem/TaskBase.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "TaskSystem/MissionManager.h"

UTaskBase::UTaskBase()
    : CurrentState(ETaskState::NotStarted)
    , ProgressPercent(0.0f)
    , CustomProgressInt(0)
    , CustomProgressFloat(0.0f)
    , CustomConfig(nullptr)
    , bEnableTick(true)
    , TickInterval(0.0f)
    , bHasNextTask(false)
    , LastTickTime(0.0f)
{
}

UTaskBase::~UTaskBase()
{
    StopTickTimer();
}

void UTaskBase::InitializeTask(const FTaskConfigRow& ConfigRow)
{
    TaskID = ConfigRow.TaskID;
    TaskName = ConfigRow.TaskName;
    SavePolicy = ConfigRow.SavePolicy;
    bAutoStart = ConfigRow.bAutoStart;
    bRepeatable = ConfigRow.bRepeatable;
    CustomConfig = ConfigRow.CustomConfig;
    bEnableTick = ConfigRow.bEnableTick;
    TickInterval = ConfigRow.TickInterval;
    bHasNextTask = ConfigRow.bHasNextTask;
    NextTaskID = ConfigRow.NextTaskID;

    UE_LOG(LogTemp, Log, TEXT("Task initialized: %s (ID: %s)"), *TaskName.ToString(), *TaskID.ToString());
}

void UTaskBase::StartTask()
{
    if (CurrentState == ETaskState::NotStarted || CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Running);
        StartTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task started: %s"), *TaskID.ToString());
        OnStartTask();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start task %s in state %d"), *TaskID.ToString(), (int32)CurrentState);
    }
}

void UTaskBase::PauseTask()
{
    if (CurrentState == ETaskState::Running)
    {
        SetState(ETaskState::Paused);
        StopTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task paused: %s"), *TaskID.ToString());
        OnPauseTask();
    }
}

void UTaskBase::ResumeTask()
{
    if (CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Running);
        StartTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task resumed: %s"), *TaskID.ToString());
        OnResumeTask();
    }
}

void UTaskBase::AbandonTask()
{
    if (CurrentState == ETaskState::Running || CurrentState == ETaskState::Paused)
    {
        StopTickTimer();
        SetState(ETaskState::Abandoned);
        UE_LOG(LogTemp, Log, TEXT("Task abandoned: %s"), *TaskID.ToString());
        OnAbandonTask();
    }
}

void UTaskBase::CompleteTask()
{
    if (CurrentState == ETaskState::Running)
    {
        StopTickTimer();
        SetState(ETaskState::Completed);
        OnCompleted.Broadcast(TaskID);
        UE_LOG(LogTemp, Log, TEXT("Task completed: %s"), *TaskID.ToString());

        if (bHasNextTask && !NextTaskID.IsNone())
        {
            UMissionManager* MissionMgr = UMissionManager::GetMissionManager();
            if (MissionMgr)
            {
                UTaskBase* NextTask = MissionMgr->CreateTask(NextTaskID);
                if (NextTask)
                {
                    UE_LOG(LogTemp, Log, TEXT("Created next task: %s after completing %s"), *NextTaskID.ToString(), *TaskID.ToString());
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Failed to create next task: %s"), *NextTaskID.ToString());
                }
            }
        }
        OnCompleteTask();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete task %s in state %d"), *TaskID.ToString(), (int32)CurrentState);
    }
}

void UTaskBase::FailTask()
{
    if (CurrentState == ETaskState::Running)
    {
        StopTickTimer();
        SetState(ETaskState::Failed);
        OnFailed.Broadcast(TaskID);
        UE_LOG(LogTemp, Log, TEXT("Task failed: %s"), *TaskID.ToString());
        OnFailTask();
    }
}

void UTaskBase::ResetTask()
{
    StopTickTimer();
    CurrentState = ETaskState::NotStarted;
    ProgressPercent = 0.0f;
    CustomProgressInt = 0;
    CustomProgressFloat = 0.0f;
    LastTickTime = 0.0f;
    OnStateChanged.Broadcast(TaskID);
    OnProgressChanged.Broadcast(TaskID, 0.0f);
    OnResetTask();

    if (bAutoStart && CurrentState == ETaskState::NotStarted)
    {
        StartTask();
    }
}

void UTaskBase::UpdateProgress(int32 DeltaInt, float DeltaFloat)
{
    if (CurrentState != ETaskState::Running)
        return;

    CustomProgressInt += DeltaInt;
    CustomProgressFloat += DeltaFloat;
    OnProgressUpdated();
}

void UTaskBase::Tick(float DeltaTime)
{
    OnTick(DeltaTime);
}

void UTaskBase::SaveTask(FTaskSaveData& OutData) const
{
    OutData.TaskID = TaskID;
    OutData.State = CurrentState;
    OutData.ProgressInt = CustomProgressInt;
    OutData.ProgressFloat = CustomProgressFloat;
    OutData.bIsCompleted = (CurrentState == ETaskState::Completed);
}

void UTaskBase::LoadTask(const FTaskSaveData& InData)
{
    CurrentState = InData.State;
    CustomProgressInt = InData.ProgressInt;
    CustomProgressFloat = InData.ProgressFloat;
    ProgressPercent = 0.0f;

    if (CurrentState == ETaskState::Running)
    {
        StartTickTimer();
        OnStateChanged.Broadcast(TaskID);
    }
}

void UTaskBase::SetState(ETaskState NewState)
{
    if (CurrentState == NewState)
        return;

    CurrentState = NewState;
    OnStateChanged.Broadcast(TaskID);
}

void UTaskBase::StartTickTimer()
{
    if (!bEnableTick)
        return;

    StopTickTimer();

    UWorld* World = GetWorld();
    if (!World)
        return;

    float ActualInterval = (TickInterval > 0.0f) ? TickInterval : 0.016f; // 默认约60fps
    World->GetTimerManager().SetTimer(TickTimerHandle, this, &UTaskBase::InternalTick, ActualInterval, true);
    LastTickTime = World->GetTimeSeconds();
}

void UTaskBase::StopTickTimer()
{
    UWorld* World = GetWorld();
    if (World && TickTimerHandle.IsValid())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }
}

void UTaskBase::InternalTick()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    if (CurrentState != ETaskState::Running)
    {
        StopTickTimer();
        return;
    }

    float CurrentTime = World->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastTickTime;
    LastTickTime = CurrentTime;

    // 限制最大 DeltaTime，避免跳跃过大
    DeltaTime = FMath::Min(DeltaTime, 0.1f);

    Tick(DeltaTime);
}

void UTaskBase::BeginDestroy()
{
    StopTickTimer();
    Super::BeginDestroy();
}

UWorld* UTaskBase::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
                return Context.World();
        }
    }
    return nullptr;
}

void UTaskBase::OnProgressUpdated_Implementation()
{
    // 子类重写实现进度逻辑
}

void UTaskBase::OnTick_Implementation(float DeltaTime)
{
    // 默认空实现，蓝图子类可选择重写
}