#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManDoubleJumpingState.generated.h"

/**
 * ������״̬ - ��ɫ�ڿ��н��еڶ�����Ծʱ��״̬
 * ����ת���������䡢����
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