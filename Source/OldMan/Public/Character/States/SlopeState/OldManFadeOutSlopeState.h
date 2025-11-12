// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/States/SlopeState/OldManOnSlopeState.h"
#include "OldManFadeOutSlopeState.generated.h"

/**
 * 
 */
UCLASS()
class OLDMAN_API UOldManFadeOutSlopeState : public UOldManOnSlopeState
{
	GENERATED_BODY()
	
public:
	virtual void Enter() override;
	virtual void Exit() override;

private:
	void CheckToMoveState();
};
