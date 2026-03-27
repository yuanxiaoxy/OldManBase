// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TaskSystem/TaskBase.h"
#include "Character/OldManCharacter.h"
#include "Task_CheckPlayerAction.generated.h"

/**
 * 
 */
UCLASS()
class OLDMAN_API UTask_CheckPlayerAction : public UTaskBase
{
	GENERATED_BODY()
public:
    virtual void InitializeTask(const FTaskConfigRow& ConfigRow) override;

protected:
    UFUNCTION(BlueprintCallable)
    AOldManCharacter* GetCachedPlayer();

private:
    AOldManCharacter* Cachedlayer;
};
