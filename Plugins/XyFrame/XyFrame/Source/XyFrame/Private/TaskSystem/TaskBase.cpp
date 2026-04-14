// TaskBase.cpp
#include "TaskSystem/TaskBase.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "TaskSystem/MissionManager.h"

UTaskBase::UTaskBase()
    : CurrentState(ETaskState::NotStarted)
    , ProgressPercent(0.0f)
    , CustomProgressInt(0)
    , CustomProgressFloat(0.0f)
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
    CustomConfig = ConfigRow.CustomConfig;  // FInstancedStruct 拷贝赋值
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
        UE_LOG(LogTemp, Log, TEXT("Task paused: %s"), *TaskID.ToString());
        OnPauseTask();
    }
}

void UTaskBase::ResumeTask()
{
    if (CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Running);
        ResetTickTimer();
        UE_LOG(LogTemp, Log, TEXT("Task resumed: %s"), *TaskID.ToString());
        OnResumeTask();
    }
}

void UTaskBase::AbandonTask()
{
    if (CurrentState == ETaskState::Running || CurrentState == ETaskState::Paused)
    {
        SetState(ETaskState::Abandoned);
        UE_LOG(LogTemp, Log, TEXT("Task abandoned: %s"), *TaskID.ToString());
        OnAbandonTask();
    }
}

void UTaskBase::CompleteTask()
{
    if (CurrentState == ETaskState::Running)
    {
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
        SetState(ETaskState::Failed);
        OnFailed.Broadcast(TaskID);
        UE_LOG(LogTemp, Log, TEXT("Task failed: %s"), *TaskID.ToString());
        OnFailTask();
    }
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

    // 注意：CustomConfig 结构体未保存，可根据需求添加序列化逻辑
    // 例如：将 CustomConfig 序列化到 OutData 的附加字段中
}

void UTaskBase::LoadTask(const FTaskSaveData& InData)
{
    CurrentState = InData.State;
    CustomProgressInt = InData.ProgressInt;
    CustomProgressFloat = InData.ProgressFloat;

    ProgressPercent = 0.0f;

    if (CurrentState == ETaskState::Running)
    {
        OnStateChanged.Broadcast(TaskID);
    }

    // CustomConfig 在 InitializeTask 时已从配置表加载，加载存档不覆盖
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

// ========== BlueprintNativeEvent 默认实现 ==========
void UTaskBase::OnStartTask_Implementation() {}
void UTaskBase::OnPauseTask_Implementation() {}
void UTaskBase::OnResumeTask_Implementation() {}
void UTaskBase::OnAbandonTask_Implementation() {}
void UTaskBase::OnCompleteTask_Implementation() {}
void UTaskBase::OnFailTask_Implementation() {}
void UTaskBase::OnResetTask_Implementation() {}
void UTaskBase::OnTick_Implementation(float DeltaTime) {}
void UTaskBase::OnProgressUpdated_Implementation() {}