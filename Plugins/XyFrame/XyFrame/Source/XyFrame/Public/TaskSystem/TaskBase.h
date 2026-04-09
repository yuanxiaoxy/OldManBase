// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TaskSystem/TaskTypes.h"
#include "TaskBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskStateChanged, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskProgressChanged, FName, TaskID, float, ProgressPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskFailed, FName, TaskID);

UCLASS(Abstract, Blueprintable, BlueprintType)
class XYFRAME_API UTaskBase : public UObject
{
    GENERATED_BODY()

public:
    UTaskBase();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void InitializeTask(const FTaskConfigRow& ConfigRow);

    // 生命周期函数（C++ 核心逻辑 + 蓝图事件）
    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void StartTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void PauseTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void ResumeTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void AbandonTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void CompleteTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void FailTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void ResetTask();

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void UpdateProgress(int32 DeltaInt, float DeltaFloat = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void Tick(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Task")
    FName GetTaskID() const { return TaskID; }

    UFUNCTION(BlueprintPure, Category = "Task")
    ETaskState GetTaskState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Task")
    float GetProgressPercent() const { return ProgressPercent; }

    UFUNCTION(BlueprintPure, Category = "Task")
    bool IsTickEnabled() const { return bEnableTick; }

    UFUNCTION(BlueprintPure, Category = "Task")
    float GetTickInterval() const { return TickInterval; }

    ETaskSavePolicy GetSavePolicy() const { return SavePolicy; }

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void SaveTask(FTaskSaveData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Task")
    virtual void LoadTask(const FTaskSaveData& InData);

    template <typename T>
    const T* GetConfigData() const
    {
        return Cast<T>(CustomConfig);
    }

    // C++ 委托（外部监听）
    UPROPERTY(BlueprintAssignable, Category = "Task")
    FOnTaskStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Task")
    FOnTaskProgressChanged OnProgressChanged;

    UPROPERTY(BlueprintAssignable, Category = "Task")
    FOnTaskCompleted OnCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Task")
    FOnTaskFailed OnFailed;

protected:
    // 以下所有蓝图事件均改为 BlueprintNativeEvent，并提供 C++ 默认实现，防止蓝图未重写时断言
    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnStartTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnPauseTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnResumeTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnAbandonTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnCompleteTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnFailTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnResetTask();

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnTick(float DeltaTime);

    UFUNCTION(BlueprintNativeEvent, Category = "Task")
    void OnProgressUpdated();

    UFUNCTION(BlueprintCallable, Category = "Task")
    void SetState(ETaskState NewState);

    UWorld* GetWorld() const override;

    void ResetTickTimer();

    // 任务标识
    FName TaskID;
    FText TaskName;

    // 保存策略
    ETaskSavePolicy SavePolicy;
    bool bAutoStart;
    bool bRepeatable;

    // Tick 控制
    bool bEnableTick;
    float TickInterval;

    // 状态
    ETaskState CurrentState;
    float ProgressPercent;

    // 自定义进度（供子类使用）
    int32 CustomProgressInt;
    float CustomProgressFloat;

    // 配置对象
    UPROPERTY()
    TObjectPtr<UTaskConfigDataBase> CustomConfig;

    // 任务链
    bool bHasNextTask;
    FName NextTaskID;

    friend class UMissionManager;
    float LastTickTime;
};