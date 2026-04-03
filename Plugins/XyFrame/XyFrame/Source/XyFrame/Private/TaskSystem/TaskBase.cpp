// Fill out your copyright notice in the Description page of Project Settings.

#include "TaskSystem/TaskBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
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
        ResetTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task started: %s"), *TaskID.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start task in state %d"), (int32)CurrentState);
        return;
    }
    OnStartTask();  // 蓝图事件
}

void UTaskBase::PauseTask()
{
    if (CurrentState == ETaskState::Running)
    {
        SetState(ETaskState::Paused);
        UE_LOG(LogTemp, Log, TEXT("Task paused: %s"), *TaskID.ToString());
    }
    OnPauseTask();
}

void UTaskBase::ResumeTask()
{
    if (CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Running);
        ResetTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task resumed: %s"), *TaskID.ToString());
    }
    OnResumeTask();
}

void UTaskBase::AbandonTask()
{
    if (CurrentState == ETaskState::Running || CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Abandoned);
        UE_LOG(LogTemp, Log, TEXT("Task abandoned: %s"), *TaskID.ToString());
    }
    OnAbandonTask();
}

void UTaskBase::CompleteTask()
{
    if (CurrentState == ETaskState::Running)
    {
        SetState(ETaskState::Completed);
        OnCompleted.Broadcast(TaskID);
        UE_LOG(LogTemp, Log, TEXT("Task completed: %s"), *TaskID.ToString());

        // 任务链
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
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete task %s in state %d"), *TaskID.ToString(), (int32)CurrentState);
    }
    OnCompleteTask();
}

void UTaskBase::FailTask()
{
    if (CurrentState == ETaskState::Running)
    {
        SetState(ETaskState::Failed);
        OnFailed.Broadcast(TaskID);
        UE_LOG(LogTemp, Log, TEXT("Task failed: %s"), *TaskID.ToString());
    }
    OnFailTask();
}

void UTaskBase::ResetTask()
{
    CurrentState = ETaskState::NotStarted;
    ProgressPercent = 0.0f;
    CustomProgressInt = 0;
    CustomProgressFloat = 0.0f;
    LastTickTime = 0.0f;
    OnStateChanged.Broadcast(TaskID);
    OnProgressChanged.Broadcast(TaskID, 0.0f);
    OnResetTask();
}

void UTaskBase::UpdateProgress(int32 DeltaInt, float DeltaFloat)
{
    if (CurrentState != ETaskState::Running)
        return;

    CustomProgressInt += DeltaInt;
    CustomProgressFloat += DeltaFloat;
    OnProgressUpdated();   // C++ 和蓝图都可重写
}

void UTaskBase::Tick(float DeltaTime)
{
    // 基类默认实现为空，但子类可重写，或蓝图重写 OnTick
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

    // 子类应重写 OnProgressUpdated 来更新 ProgressPercent
    ProgressPercent = 0.0f;

    if (CurrentState == ETaskState::Running)
    {
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

void UTaskBase::ResetTickTimer()
{
    UWorld* World = GetWorld();
    if (World)
    {
        LastTickTime = World->GetTimeSeconds();
    }
    else
    {
        LastTickTime = 0.0f;
    }
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

// OnProgressUpdated 默认实现（空）
void UTaskBase::OnProgressUpdated_Implementation()
{
    // 子类重写实现进度逻辑
}