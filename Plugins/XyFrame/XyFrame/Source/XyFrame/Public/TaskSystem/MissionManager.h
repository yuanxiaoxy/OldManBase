// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "TaskSystem/TaskTypes.h"
#include "TaskSystem/TaskBase.h"
#include "MissionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskAdded, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskRemoved, FName, TaskID);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UMissionManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UMissionManager)

public:
    UMissionManager();
    virtual ~UMissionManager() override;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "MissionManager", meta = (DisplayName = "Get Mission Manager"))
    static UMissionManager* GetMissionManager() { return GetInstance(); }

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    bool LoadTaskTable(UDataTable* TaskTable);

    // 创建任务：若已存在且可重复（且处于终态），则先移除旧任务再创建新任务；
    // 若已存在且正在运行/暂停，则返回 nullptr 并输出警告。
    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    UTaskBase* CreateTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void StartTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void CompleteTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void FailTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void AbandonTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void PauseTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void ResumeTask(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void ResetTask(FName TaskID);

    UFUNCTION(BlueprintPure, Category = "MissionManager")
    UTaskBase* GetTask(FName TaskID) const;

    UFUNCTION(BlueprintPure, Category = "MissionManager")
    TArray<UTaskBase*> GetAllTasks() const;

    UFUNCTION(BlueprintPure, Category = "MissionManager")
    TArray<UTaskBase*> GetTasksByState(ETaskState State) const;

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void SaveAllPersistentTasks();

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void LoadAllPersistentTasks();

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void ClearSessionTasks();

    UFUNCTION(BlueprintCallable, Category = "MissionManager")
    void ClearAllTasks();

    UPROPERTY(BlueprintAssignable, Category = "MissionManager")
    FOnTaskAdded OnTaskAdded;

    UPROPERTY(BlueprintAssignable, Category = "MissionManager")
    FOnTaskRemoved OnTaskRemoved;

protected:
    UFUNCTION()
    void UpdateTasks();

private:
    UTaskBase* CreateTaskInstance(const FTaskConfigRow& ConfigRow);
    void RemoveAndDestroyTask(FName TaskID);
    void SavePersistentTaskIfNeeded(UTaskBase* Task);

    void StartUpdateTimer();
    void StopUpdateTimer();

    // 使用弱指针存储任务，防止悬空指针
    TMap<FName, TWeakObjectPtr<UTaskBase>> ActiveTasks;
    TMap<FName, FTaskConfigRow> TaskConfigs;

    FTimerHandle UpdateTimerHandle;
    bool bIsUpdateTimerActive;

    UWorld* GetWorld() const override;
};