// Task_CheckDraggableItem.h
#pragma once

#include "CoreMinimal.h"
#include "TaskSystem/TaskBase.h"
#include "DraggableSplineActor/DraggableSplineActor.h"
#include "Task_CheckDraggableItem.generated.h"

// 配置结构体，继承自 FTaskConfigBase
USTRUCT(BlueprintType)
struct OLDMAN_API FCheckDraggableItemConfig : public FTaskConfigBase
{
    GENERATED_BODY()

public:
    // 是否需要拖动超过最小距离
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CheckDraggable")
    bool bRequireMinDistance = false;

    // 最小拖动距离（样条位置变化量，0~1）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CheckDraggable", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bRequireMinDistance"))
    float MinDeltaPosition = 0.1f;
};

/**
 * 检测任意可拖动物体是否被拖动的任务（监听 Manager 全局事件）
 */
UCLASS()
class OLDMAN_API UTask_CheckDraggableItem : public UTaskBase
{
    GENERATED_BODY()

public:
    virtual void InitializeTask(const FTaskConfigRow& ConfigRow) override;
    virtual void StartTask() override;
    virtual void OnCompleteTask_Implementation() override;
    virtual void OnFailTask_Implementation() override;
    virtual void OnAbandonTask_Implementation() override;

private:
    // 配置数据
    bool bRequireMinDistance = false;
    float MinDeltaPosition = 0.1f;

    // 任务状态
    bool bHasCompleted = false;

    // 记录每个 Actor 拖动开始时的位置（弱指针 Key，起始位置 Value）
    TMap<TWeakObjectPtr<ADraggableSplineActor>, float> DragStartPositions;

    // Manager 引用
    UPROPERTY()
    TObjectPtr<class ADraggableSplineActorManager> CachedManager;

    // 委托回调
    UFUNCTION()
    void OnAnyDraggingStarted(ADraggableSplineActor* DraggedActor);

    UFUNCTION()
    void OnAnyDraggingStopped(ADraggableSplineActor* DraggedActor);
};