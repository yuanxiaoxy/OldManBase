// Fill out your copyright notice in the Description page of Project Settings.

#include "TaskSystem/ExampleTask/KillTask.h"
#include "TaskSystem/TaskTypes.h"

void UKillTask::InitializeTask(const FTaskConfigRow& ConfigRow)
{
    Super::InitializeTask(ConfigRow);

    /*const UKillTaskConfig* KillConfig = GetConfigData<UKillTaskConfig>();
    if (KillConfig)
    {
        RequiredKills = KillConfig->RequiredKills;
    }
    else
    {
        RequiredKills = 5;
    }*/

    // 直接访问基类 protected 成员
    CustomProgressInt = 0;
    ProgressPercent = 0.0f;
}

void UKillTask::OnEnemyKilled()
{
    if (CurrentState != ETaskState::Running)
        return;

    UpdateProgress(1, 0.0f);
}

void UKillTask::OnProgressUpdated_Implementation()
{
    // 访问基类 protected 成员
    ProgressPercent = (float)CustomProgressInt / (float)RequiredKills;
    OnProgressChanged.Broadcast(TaskID, ProgressPercent);

    if (CustomProgressInt >= RequiredKills)
    {
        CompleteTask();  // 调用 C++ 的 CompleteTask，它自动触发委托、状态变更、任务链，并调用 OnCompleteTask 蓝图事件
        UE_LOG(LogTemp, Log, TEXT("KillTask completed! Killed %d enemies."), RequiredKills);
    }
}