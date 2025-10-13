#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManJumpingState.generated.h"

/**
 * 跳跃状态 - 角色第一次跳跃时的状态
 * 可以转换到：下落、二段跳、死亡
 */
UCLASS()
class OLDMAN_API UOldManJumpingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	void CheckStateTransitions(class AOldManCharacter* Character);
};