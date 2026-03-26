// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SaveManager/SaveGameTool.h"
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

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class XYFRAME_API UTaskConfigDataBase : public UObject
{
    GENERATED_BODY()
public:
};

UCLASS(Blueprintable, EditInlineNew)
class XYFRAME_API UKillTaskConfig : public UTaskConfigDataBase
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KillTask")
    int32 RequiredKills = 5;
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "Task")
    TObjectPtr<UTaskConfigDataBase> CustomConfig = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
    bool bEnableTick = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bEnableTick"))
    float TickInterval = 0.0f;

    // 任务链：完成后是否有下一个任务
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Chain")
    bool bHasNextTask = false;

    // 下一个任务ID（当 bHasNextTask 为 true 时有效）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Chain", meta = (EditCondition = "bHasNextTask"))
    FName NextTaskID;
};

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