// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/States/OldManStateBase.h"
#include "OldManOnSlopeState.generated.h"

/**
 * 
 */
UCLASS()
class OLDMAN_API UOldManOnSlopeState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

protected:
	virtual void SetupTransitionRules() override;

	virtual void HandleMovement(float DeltaTime) override;
	virtual void HandleRotation(float DeltaTime) override;
};
