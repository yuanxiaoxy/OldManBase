#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManStateBase.h"
#include "OldManLandState.generated.h"

/**
 * ���״̬ - ��ɫ�ӿ������ʱ�Ķ���״̬
 * ����ת������վ�������ߡ��ܲ�������
 */
UCLASS()
class OLDMAN_API UOldManLandState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

private:
	float LandStartTime;
	float LandDuration;

	void CheckStateTransitions();
};