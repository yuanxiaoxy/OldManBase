#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManAttackingState.generated.h"

/**
 * 攻击状态 - 角色进行攻击时的状态
 * 可以转换到：站立、行走、跑步、死亡
 */
UCLASS()
class OLDMAN_API UOldManAttackingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	float AttackStartTime;
	float AttackDuration;

	void CheckStateTransitions();
	void PerformAttack(class AOldManCharacter* Character);
};