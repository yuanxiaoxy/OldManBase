#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManRunningState.generated.h"

/**
 * 跑步状态 - 角色快速移动时的状态
 * 可以转换到：站立、行走、跳跃、攻击、死亡
 */
UCLASS()
class OLDMAN_API UOldManRunningState : public UOldManStateBase
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