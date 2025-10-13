#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManDeadState.generated.h"

/**
 * ����״̬ - ��ɫ����ʱ��״̬
 * ��������״̬������ת��������״̬
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