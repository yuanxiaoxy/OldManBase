#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManDoubleJumpingState.generated.h"

/**
 * 二段跳状态 - 角色在空中进行第二次跳跃时的状态
 * 可以转换到：下落、死亡
 */
UCLASS()
class OLDMAN_API UOldManDoubleJumpingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	void CheckStateTransitions(class AOldManCharacter* Character);
};