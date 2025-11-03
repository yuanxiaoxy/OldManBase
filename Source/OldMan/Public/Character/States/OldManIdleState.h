#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManIdleState.generated.h"

/**
 * 站立状态 - 角色静止时的状态
 * 可以转换到：行走、跑步、跳跃、攻击、死亡
 */
UCLASS()
class OLDMAN_API UOldManIdleState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	// 检查状态转换条件
	void CheckStateTransitions();
};