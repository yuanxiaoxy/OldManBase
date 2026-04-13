// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TaskSystem/TaskBase.h"
#include "KillTask.generated.h"

UCLASS(Blueprintable)
class XYFRAME_API UKillTask : public UTaskBase
{
    GENERATED_BODY()

public:
    virtual void InitializeTask(const FTaskConfigRow& ConfigRow) override;

    UFUNCTION(BlueprintCallable, Category = "KillTask")
    void OnEnemyKilled();

    // 重写 OnProgressUpdated（C++ 版本）
    virtual void OnProgressUpdated_Implementation() override;


    // 示例：获取击杀任务配置（蓝图可调用）
    UFUNCTION(BlueprintPure, Category = "Task|Config")
    bool GetKillConfig(int32& OutRequiredKills, FName& OutTargetEnemyType) const
    {
        if (const FKillTaskConfig* Config = CustomConfig.GetPtr<FKillTaskConfig>())
        {
            OutRequiredKills = Config->RequiredKills;
            OutTargetEnemyType = Config->TargetEnemyType;
            return true;
        }
        return false;
    }

private:
    int32 RequiredKills;
};