// Task_CheckDraggableItem.cpp
#include "Task/Task_CheckDraggableItem.h"
#include "TaskSystem/TaskTypes.h"
#include "DraggableSplineActor/DraggableSplineActorManager.h"
#include "Engine/World.h"

void UTask_CheckDraggableItem::InitializeTask(const FTaskConfigRow& ConfigRow)
{
    Super::InitializeTask(ConfigRow);

    // 获取配置数据
    const FCheckDraggableItemConfig* Config = GetConfigData<FCheckDraggableItemConfig>();
    if (Config)
    {
        bRequireMinDistance = Config->bRequireMinDistance;
        MinDeltaPosition = Config->MinDeltaPosition;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Task %s: CustomConfig is not of type FCheckDraggableItemConfig"), *TaskID.ToString());
    }
}

void UTask_CheckDraggableItem::StartTask()
{
    Super::StartTask();

    // 获取单例 Manager
    CachedManager = ADraggableSplineActorManager::GetInstance();
    if (!CachedManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Task %s: DraggableSplineActorManager not available"), *TaskID.ToString());
        FailTask();
        return;
    }

    // 绑定全局拖动事件
    CachedManager->OnAnyDraggingStarted.AddDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStarted);
    CachedManager->OnAnyDraggingStopped.AddDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStopped);

    bHasCompleted = false;
    DragStartPositions.Empty();

    UE_LOG(LogTemp, Log, TEXT("Task %s: Listening to all draggable actors via Manager"), *TaskID.ToString());
}

void UTask_CheckDraggableItem::OnAnyDraggingStarted(ADraggableSplineActor* DraggedActor)
{
    // 任务未运行或已标记完成则忽略
    if (bHasCompleted || GetTaskState() != ETaskState::Running)
        return;

    if (!DraggedActor || !IsValid(DraggedActor))
        return;

    // 记录该 Actor 拖动开始时的样条位置
    float StartPos = DraggedActor->GetCurrentSplinePosition();
    DragStartPositions.Add(TWeakObjectPtr<ADraggableSplineActor>(DraggedActor), StartPos);
}

void UTask_CheckDraggableItem::OnAnyDraggingStopped(ADraggableSplineActor* DraggedActor)
{
    // 任务未运行或已标记完成则忽略
    if (bHasCompleted || GetTaskState() != ETaskState::Running)
        return;

    if (!DraggedActor || !IsValid(DraggedActor))
        return;

    // 查找该 Actor 的起始位置记录
    TWeakObjectPtr<ADraggableSplineActor> WeakActor(DraggedActor);
    float* StartPosPtr = DragStartPositions.Find(WeakActor);
    if (!StartPosPtr)
    {
        // 如果没有记录（可能未正确触发开始事件），则忽略本次停止
        return;
    }

    float StartPosition = *StartPosPtr;
    float CurrentPosition = DraggedActor->GetCurrentSplinePosition();

    // 判断是否满足完成条件
    bool bShouldComplete = false;
    if (!bRequireMinDistance)
    {
        // 不需要最小距离：只要被拖动过就完成
        bShouldComplete = true;
    }
    else
    {
        float DeltaPos = FMath::Abs(CurrentPosition - StartPosition);
        if (DeltaPos >= MinDeltaPosition)
        {
            bShouldComplete = true;
        }
    }

    if (bShouldComplete)
    {
        bHasCompleted = true;
        CompleteTask();
    }

    // 清除该 Actor 的记录（无论是否完成，本次拖动周期结束）
    DragStartPositions.Remove(WeakActor);
}

void UTask_CheckDraggableItem::OnCompleteTask_Implementation()
{
    // 解绑事件
    if (CachedManager)
    {
        CachedManager->OnAnyDraggingStarted.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStarted);
        CachedManager->OnAnyDraggingStopped.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStopped);
    }

    DragStartPositions.Empty();
    Super::OnCompleteTask_Implementation();
}

void UTask_CheckDraggableItem::OnFailTask_Implementation()
{
    // 失败时也清理事件绑定
    if (CachedManager)
    {
        CachedManager->OnAnyDraggingStarted.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStarted);
        CachedManager->OnAnyDraggingStopped.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStopped);
    }

    DragStartPositions.Empty();
    Super::OnFailTask_Implementation();
}

void UTask_CheckDraggableItem::OnAbandonTask_Implementation()
{
    // 放弃时也清理
    if (CachedManager)
    {
        CachedManager->OnAnyDraggingStarted.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStarted);
        CachedManager->OnAnyDraggingStopped.RemoveDynamic(this, &UTask_CheckDraggableItem::OnAnyDraggingStopped);
    }

    DragStartPositions.Empty();
    Super::OnAbandonTask_Implementation();
}