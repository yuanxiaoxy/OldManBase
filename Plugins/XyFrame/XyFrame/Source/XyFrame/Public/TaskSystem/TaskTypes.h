// TaskTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SaveManager/SaveGameTool.h"
#include "InstancedStruct.h" // 新增
#include "TaskTypes.generated.h"

class UTaskBase;

UENUM(BlueprintType)
enum class ETaskState : uint8
{
    NotStarted   UMETA(DisplayName = "未开始"),
    Running      UMETA(DisplayName = "进行中"),
    Paused       UMETA(DisplayName = "暂停"),
    Completed    UMETA(DisplayName = "已完成"),
    Failed       UMETA(DisplayName = "失败"),
    Abandoned    UMETA(DisplayName = "放弃")
};

UENUM(BlueprintType)
enum class ETaskSavePolicy : uint8
{
    NoSave        UMETA(DisplayName = "不保存（可重复触发）"),
    SessionOnly   UMETA(DisplayName = "单局保存（退出重置）"),
    Persistent    UMETA(DisplayName = "永久保存（本地持久化）")
};

// ========== 结构体配置体系（替代原 UObject 体系） ==========

/** 任务配置基础结构体，所有自定义配置必须继承自此结构体 */
USTRUCT(BlueprintType)
struct XYFRAME_API FTaskConfigBase
{
    GENERATED_BODY()
};

/** 示例：击杀任务配置结构体 */
USTRUCT(BlueprintType)
struct XYFRAME_API FKillTaskConfig : public FTaskConfigBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KillTask")
    int32 RequiredKills = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KillTask")
    FName TargetEnemyType;
};

/** 示例：收集任务配置结构体 */
USTRUCT(BlueprintType)
struct XYFRAME_API FCollectTaskConfig : public FTaskConfigBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollectTask")
    FName ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CollectTask")
    int32 RequiredAmount = 1;
};

// ========== DataTable 行结构 ==========

USTRUCT(BlueprintType)
struct FTaskConfigRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FName TaskID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FText TaskName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    ETaskSavePolicy SavePolicy = ETaskSavePolicy::SessionOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bAutoStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    TSubclassOf<UTaskBase> TaskClass;

    // 使用 FInstancedStruct 替代原来的 TObjectPtr<UTaskConfigDataBase>
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (BaseStruct = "/Script/XYFrame.TaskConfigBase"))
    FInstancedStruct CustomConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bEnableTick = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bEnableTick"))
    float TickInterval = 0.0f;

    // 任务链
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Chain")
    bool bHasNextTask = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Chain", meta = (EditCondition = "bHasNextTask"))
    FName NextTaskID;
};

// ========== 保存数据结构（不变） ==========

USTRUCT(BlueprintType)
struct FTaskSaveData
{
    GENERATED_BODY()

    UPROPERTY()
    FName TaskID;

    UPROPERTY()
    ETaskState State;

    UPROPERTY()
    int32 ProgressInt = 0;

    UPROPERTY()
    float ProgressFloat = 0.0f;

    UPROPERTY()
    bool bIsCompleted = false;
};

UCLASS(BlueprintType)
class XYFRAME_API UTaskSaveGame : public USaveGameBase
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SaveGame")
    TArray<FTaskSaveData> PersistentTasks;
};