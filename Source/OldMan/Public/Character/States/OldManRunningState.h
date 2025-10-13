#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManRunningState.generated.h"

/**
 * �ܲ�״̬ - ��ɫ�����ƶ�ʱ��״̬
 * ����ת������վ�������ߡ���Ծ������������
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