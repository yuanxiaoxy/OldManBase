#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManFallingState.generated.h"

/**
 * 下落状态 - 角色在空中下落时的状态
 * 可以转换到：落地、二段跳、死亡
 */
UCLASS()
class OLDMAN_API UOldManFallingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	void CheckStateTransitions();
};