#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManWalkingState.generated.h"

/**
 * 行走状态 - 角色正常移动时的状态
 * 可以转换到：站立、跑步、跳跃、攻击、死亡
 */
UCLASS()
class OLDMAN_API UOldManWalkingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	void CheckStateTransitions(class AOldManCharacter* Character);
	void UpdateAnimation(class AOldManCharacter* Character);
};