#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManWalkingState.generated.h"

/**
 * ����״̬ - ��ɫ�����ƶ�ʱ��״̬
 * ����ת������վ�����ܲ�����Ծ������������
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