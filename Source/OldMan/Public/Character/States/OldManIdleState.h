#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManIdleState.generated.h"

/**
 * վ��״̬ - ��ɫ��ֹʱ��״̬
 * ����ת���������ߡ��ܲ�����Ծ������������
 */
UCLASS()
class OLDMAN_API UOldManIdleState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	// ���״̬ת������
	void CheckStateTransitions();
};