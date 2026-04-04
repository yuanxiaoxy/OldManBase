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
    // 检查是否已存在
    if (UTaskBase** ExistingTaskPtr = ActiveTasks.Find(TaskID))
    {
        UTaskBase* ExistingTask = *ExistingTaskPtr;
        if (!ExistingTask || !IsValid(ExistingTask))
        {
            // 无效对象，直接移除
            ActiveTasks.Remove(TaskID);
        }
        else
        {
            const FTaskConfigRow* Config = TaskConfigs.Find(TaskID);
            bool bRepeatable = Config ? Config->bRepeatable : false;

            ETaskState State = ExistingTask->GetTaskState();
            bool bCanRecreate = bRepeatable && (State == ETaskState::Completed || State == ETaskState::Failed || State == ETaskState::Abandoned);

            if (!bCanRecreate)
            {
                UE_LOG(LogTemp, Warning, TEXT("Cannot create task %s: already exists with state %d and repeatable=%d"), *TaskID.ToString(), (int32)State, bRepeatable);
                return nullptr;
            }

            // 可重复且处于终态：先移除旧任务
            UE_LOG(LogTemp, Log, TEXT("Recreating repeatable task %s (old state: %d)"), *TaskID.ToString(), (int32)State);
            RemoveAndDestroyTask(TaskID);
        }
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

void UMissionManager::RemoveAndDestroyTask(FName TaskID)
{
    UTaskBase* Task = ActiveTasks.FindRef(TaskID);
    if (Task)
    {
        if (Task->GetTaskState() == ETaskState::Running || Task->GetTaskState() == ETaskState::Paused)
        {
            Task->AbandonTask();
        }
        ActiveTasks.Remove(TaskID);
        OnTaskRemoved.Broadcast(TaskID);
        Task->ConditionalBeginDestroy();
    }
}

void UMissionManager::SavePersistentTaskIfNeeded(UTaskBase* Task)
{
    if (!Task || Task->GetSavePolicy() != ETaskSavePolicy::Persistent)
        return;

    // 增量保存：只保存当前这个任务，避免全量写入频繁
    USaveGameTool* SaveTool = USaveGameTool::GetSaveGameTool();
    if (!SaveTool)
        return;

    UTaskSaveGame* TaskSaveGame = Cast<UTaskSaveGame>(SaveTool->LoadGameSync(TEXT("PersistentTasks")));
    if (!TaskSaveGame)
    {
        TaskSaveGame = NewObject<UTaskSaveGame>();
    }

    // 移除旧数据
    TaskSaveGame->PersistentTasks.RemoveAll([Task](const FTaskSaveData& Data) { return Data.TaskID == Task->GetTaskID(); });

    // 添加新数据
    FTaskSaveData Data;
    Task->SaveTask(Data);
    TaskSaveGame->PersistentTasks.Add(Data);

    SaveTool->SaveGameSync(TEXT("PersistentTasks"), TaskSaveGame);
}

void UMissionManager::StartTask(FName TaskID)
{
    if (UTaskBase* Task = GetTask(TaskID))
        Task->StartTask();
}

void UMissionManager::CompleteTask(FName TaskID)
{
    UTaskBase* Task = GetTask(TaskID);
    if (Task)
    {
        Task->CompleteTask();
        SavePersistentTaskIfNeeded(Task);
    }
}

void UMissionManager::FailTask(FName TaskID)
{
    UTaskBase* Task = GetTask(TaskID);
    if (Task)
    {
        Task->FailTask();
        SavePersistentTaskIfNeeded(Task);
    }
}

void UMissionManager::AbandonTask(FName TaskID)
{
    UTaskBase* Task = GetTask(TaskID);
    if (Task)
    {
        Task->AbandonTask();
        SavePersistentTaskIfNeeded(Task);
        RemoveAndDestroyTask(TaskID);
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
    TArray<FName> TaskIDs;
    for (const auto& Pair : ActiveTasks)
    {
        if (Pair.Value && Pair.Value->GetSavePolicy() == ETaskSavePolicy::SessionOnly)
        {
            TaskIDs.Add(Pair.Key);
        }
    }

    for (const FName& TaskID : TaskIDs)
    {
        RemoveAndDestroyTask(TaskID);
    }
}

void UMissionManager::ClearAllTasks()
{
    TArray<FName> TaskIDs;
    for (const auto& Pair : ActiveTasks)
    {
        TaskIDs.Add(Pair.Key);
    }

    for (const FName& TaskID : TaskIDs)
    {
        RemoveAndDestroyTask(TaskID);
    }
}

void UMissionManager::UpdateTasks()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    float DeltaTime = World->GetDeltaSeconds();
    float CurrentTime = World->GetTimeSeconds();

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