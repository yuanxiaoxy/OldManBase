// Fill out your copyright notice in the Description page of Project Settings.

#include "TaskSystem/MissionManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SaveManager/SaveGameTool.h"
#include "TaskSystem/TaskTypes.h"
#include "TaskSystem/TaskBase.h"

template<>
UMissionManager* TSingleton<UMissionManager>::SingletonInstance = nullptr;

UMissionManager::UMissionManager()
    : bIsUpdateTimerActive(false)
{
}

UMissionManager::~UMissionManager()
{
    StopUpdateTimer();
    // 析构时只清空容器，不调用任务方法
    ActiveTasks.Empty();
}

void UMissionManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("MissionManager initialized"));
    StartUpdateTimer();
    LoadAllPersistentTasks();
}

bool UMissionManager::LoadTaskTable(UDataTable* TaskTable)
{
    if (!TaskTable)
    {
        UE_LOG(LogTemp, Error, TEXT("LoadTaskTable: TaskTable is null"));
        return false;
    }

    TaskConfigs.Empty();

    static const FString ContextString(TEXT("LoadTaskTable"));
    TArray<FTaskConfigRow*> Rows;
    TaskTable->GetAllRows<FTaskConfigRow>(ContextString, Rows);

    for (FTaskConfigRow* Row : Rows)
    {
        if (Row && !Row->TaskID.IsNone())
        {
            TaskConfigs.Add(Row->TaskID, *Row);
            UE_LOG(LogTemp, Log, TEXT("Loaded task config: %s"), *Row->TaskID.ToString());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d task configs"), TaskConfigs.Num());
    return TaskConfigs.Num() > 0;
}

UTaskBase* UMissionManager::CreateTask(FName TaskID)
{
    if (ActiveTasks.Contains(TaskID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Task already exists: %s"), *TaskID.ToString());
        return nullptr;
    }

    const FTaskConfigRow* Config = TaskConfigs.Find(TaskID);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("Task config not found: %s"), *TaskID.ToString());
        return nullptr;
    }

    UTaskBase* NewTask = CreateTaskInstance(*Config);
    if (!NewTask)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create task instance for %s"), *TaskID.ToString());
        return nullptr;
    }

    ActiveTasks.Add(TaskID, NewTask);
    OnTaskAdded.Broadcast(TaskID);

    if (Config->bAutoStart)
    {
        NewTask->StartTask();
    }

    UE_LOG(LogTemp, Log, TEXT("Created task: %s"), *TaskID.ToString());
    return NewTask;
}

UTaskBase* UMissionManager::CreateTaskInstance(const FTaskConfigRow& ConfigRow)
{
    if (!ConfigRow.TaskClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Task class not specified for task %s"), *ConfigRow.TaskID.ToString());
        return nullptr;
    }

    UClass* TaskClass = ConfigRow.TaskClass;
    if (TaskClass->HasAnyClassFlags(CLASS_Abstract))
    {
        UE_LOG(LogTemp, Error, TEXT("Task class %s is abstract"), *TaskClass->GetName());
        return nullptr;
    }

    UTaskBase* NewTask = NewObject<UTaskBase>(this, TaskClass);
    NewTask->InitializeTask(ConfigRow);
    return NewTask;
}

void UMissionManager::StartTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->StartTask();
}

void UMissionManager::CompleteTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->CompleteTask();
}

void UMissionManager::FailTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->FailTask();
}

void UMissionManager::AbandonTask(FName TaskID)
{
    UTaskBase* Task = GetTask(TaskID);
    if (Task)
    {
        Task->AbandonTask();
        ActiveTasks.Remove(TaskID);
        OnTaskRemoved.Broadcast(TaskID);
        Task->ConditionalBeginDestroy();
    }
}

void UMissionManager::PauseTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->PauseTask();
}

void UMissionManager::ResumeTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->ResumeTask();
}

void UMissionManager::ResetTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->ResetTask();
}

UTaskBase* UMissionManager::GetTask(FName TaskID) const
{
    UTaskBase* const* TaskPtr = ActiveTasks.Find(TaskID);
    return TaskPtr ? *TaskPtr : nullptr;
}

TArray<UTaskBase*> UMissionManager::GetAllTasks() const
{
    TArray<UTaskBase*> Result;
    ActiveTasks.GenerateValueArray(Result);
    return Result;
}

TArray<UTaskBase*> UMissionManager::GetTasksByState(ETaskState State) const
{
    TArray<UTaskBase*> Result;
    for (const auto& Pair : ActiveTasks)
    {
        if (Pair.Value && Pair.Value->GetTaskState() == State)
            Result.Add(Pair.Value);
    }
    return Result;
}

void UMissionManager::SaveAllPersistentTasks()
{
    TArray<FTaskSaveData> SaveDataArray;
    for (const auto& Pair : ActiveTasks)
    {
        if (Pair.Value && Pair.Value->GetSavePolicy() == ETaskSavePolicy::Persistent)
        {
            FTaskSaveData Data;
            Pair.Value->SaveTask(Data);
            SaveDataArray.Add(Data);
        }
    }

    USaveGameTool* SaveTool = USaveGameTool::GetSaveGameTool();
    if (SaveTool)
    {
        UTaskSaveGame* TaskSaveGame = NewObject<UTaskSaveGame>();
        TaskSaveGame->PersistentTasks = SaveDataArray;
        SaveTool->SaveGameSync(TEXT("PersistentTasks"), TaskSaveGame);
    }
}

void UMissionManager::LoadAllPersistentTasks()
{
    USaveGameTool* SaveTool = USaveGameTool::GetSaveGameTool();
    if (!SaveTool)
        return;

    UTaskSaveGame* SaveGameObj = Cast<UTaskSaveGame>(SaveTool->LoadGameSync(TEXT("PersistentTasks")));
    if (!SaveGameObj)
        return;

    for (const FTaskSaveData& Data : SaveGameObj->PersistentTasks)
    {
        if (ActiveTasks.Contains(Data.TaskID))
            continue;

        const FTaskConfigRow* Config = TaskConfigs.Find(Data.TaskID);
        if (!Config)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot load task %s: config not found"), *Data.TaskID.ToString());
            continue;
        }

        if (Config->SavePolicy != ETaskSavePolicy::Persistent)
            continue;

        UTaskBase* Task = CreateTaskInstance(*Config);
        if (Task)
        {
            Task->LoadTask(Data);
            ActiveTasks.Add(Data.TaskID, Task);
            OnTaskAdded.Broadcast(Data.TaskID);
        }
    }
}

void UMissionManager::ClearSessionTasks()
{
    // 收集所有需要清除的任务ID
    TArray<FName> TaskIDs;
    for (const auto& Pair : ActiveTasks)
    {
        if (Pair.Value && Pair.Value->GetSavePolicy() == ETaskSavePolicy::SessionOnly)
        {
            TaskIDs.Add(Pair.Key);
        }
    }

    // 先将所有任务状态改为非运行
    for (const FName& TaskID : TaskIDs)
    {
        UTaskBase* Task = ActiveTasks.FindRef(TaskID);
        if (Task && IsValid(Task))
        {
            if (Task->GetTaskState() == ETaskState::Running || Task->GetTaskState() == ETaskState::Paused)
            {
                Task->AbandonTask();
            }
        }
    }

    // 移出并销毁
    for (const FName& TaskID : TaskIDs)
    {
        UTaskBase* Task = ActiveTasks.FindRef(TaskID);
        if (Task)
        {
            ActiveTasks.Remove(TaskID);
            OnTaskRemoved.Broadcast(TaskID);
            Task->ConditionalBeginDestroy();
        }
    }
}

void UMissionManager::ClearAllTasks()
{
    // 收集所有任务ID
    TArray<FName> TaskIDs;
    for (const auto& Pair : ActiveTasks)
    {
        TaskIDs.Add(Pair.Key);
    }

    // 先将所有任务状态改为非运行
    for (const FName& TaskID : TaskIDs)
    {
        UTaskBase* Task = ActiveTasks.FindRef(TaskID);
        if (Task && IsValid(Task))
        {
            if (Task->GetTaskState() == ETaskState::Running || Task->GetTaskState() == ETaskState::Paused)
            {
                Task->AbandonTask();
            }
        }
    }

    // 移出并销毁
    TMap<FName, UTaskBase*> TasksToDestroy = MoveTemp(ActiveTasks);
    ActiveTasks.Empty();

    for (const FName& TaskID : TaskIDs)
    {
        UTaskBase* Task = TasksToDestroy.FindRef(TaskID);
        if (Task && IsValid(Task))
        {
            OnTaskRemoved.Broadcast(TaskID);
            Task->ConditionalBeginDestroy();
        }
    }
}

void UMissionManager::UpdateTasks()
{
    float CurrentTime = 0.0f;
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentTime = World->GetTimeSeconds();
    }
    float DeltaTime = World ? World->GetDeltaSeconds() : 0.016f;

    TArray<UTaskBase*> Tasks = GetAllTasks();
    for (UTaskBase* Task : Tasks)
    {
        if (!Task || !IsValid(Task))
            continue;

        if (Task->GetTaskState() != ETaskState::Running)
            continue;

        if (!Task->IsTickEnabled())
            continue;

        float TickInterval = Task->GetTickInterval();
        bool bShouldTick = false;

        if (TickInterval <= 0.0f)
        {
            bShouldTick = true;
        }
        else
        {
            float TimeSinceLastTick = CurrentTime - Task->LastTickTime;
            if (TimeSinceLastTick >= TickInterval)
            {
                bShouldTick = true;
                Task->LastTickTime = CurrentTime;
            }
        }

        if (bShouldTick && IsValid(Task))
        {
            Task->Tick(DeltaTime);
        }
    }
}

void UMissionManager::StartUpdateTimer()
{
    if (!bIsUpdateTimerActive && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UMissionManager::UpdateTasks, 0.016f, true);
        bIsUpdateTimerActive = true;
        UE_LOG(LogTemp, Log, TEXT("MissionManager update timer started"));
    }
}

void UMissionManager::StopUpdateTimer()
{
    if (bIsUpdateTimerActive && GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
        bIsUpdateTimerActive = false;
        UE_LOG(LogTemp, Log, TEXT("MissionManager update timer stopped"));
    }
}

UWorld* UMissionManager::GetWorld() const
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