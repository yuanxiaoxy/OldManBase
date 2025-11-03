#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManDeadState.generated.h"

/**
 * 死亡状态 - 角色死亡时的状态
 * 这是最终状态，不能转换到其他状态
 */
UCLASS()
class OLDMAN_API UOldManDeadState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	void HandleDeath(class AOldManCharacter* Character);
};