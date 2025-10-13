#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManJumpingState.generated.h"

/**
 * ��Ծ״̬ - ��ɫ��һ����Ծʱ��״̬
 * ����ת���������䡢������������
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