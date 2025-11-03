#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManLandState.generated.h"

/**
 * 落地状态 - 角色从空中落地时的短暂状态
 * 可以转换到：站立、行走、跑步、死亡
 */
UCLASS()
class OLDMAN_API UOldManLandState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	float LandStartTime;
	float LandDuration;

	void CheckStateTransitions();
};