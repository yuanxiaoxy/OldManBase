#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManAttackingState.generated.h"

/**
 * ����״̬ - ��ɫ���й���ʱ��״̬
 * ����ת������վ�������ߡ��ܲ�������
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