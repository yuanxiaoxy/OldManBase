// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "OldManStateBase.generated.h"

/**
 * 
 */
UCLASS()
class OLDMAN_API UOldManStateBase : public UStateBase
{
	GENERATED_BODY()
	
public:
	virtual void Enter() override;
	virtual void Exit() override;

	void CheckJumpStatesTranisition();
	virtual void CheckAttackStatesTranisition();

public:
	void InPatchEvents();
	void OutPatchEvents();

private:
	FName Key_CheckJumpStatesTranisition = "key_CheckJumpStatesTranisition";
	FName Key_CheckAttackStatesTranisition = "key_CheckAttackStatesTranisition";
};
