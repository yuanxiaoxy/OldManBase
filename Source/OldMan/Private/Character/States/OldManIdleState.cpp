#include "Character/States/OldManIdleState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManIdleState::Enter()
{
	// վ��״̬�����߼�
	UE_LOG(LogTemp, Log, TEXT("Entering Idle State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// �����ƶ��ٶȵ������ٶ�
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->WalkSpeed;
		}

		// ������ͼ�����¼�
		Character->PlayMoveAnimation(0.0f, 0.0f);
	}
}

void UOldManIdleState::Exit()
{
	// վ��״̬�˳��߼�
	UE_LOG(LogTemp, Log, TEXT("Exiting Idle State"));
}

void UOldManIdleState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManIdleState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		// ת��������״̬
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	if (Character->IsFalling())
	{
		// ת��������״̬
		CheckTransition(UOldManFallingState::StaticClass());
		return;
	}

	if (Character->IsMoving())
	{
		if (Character->bIsRunning)
		{
			// ת�����ܲ�״̬
			CheckTransition(UOldManRunningState::StaticClass());
		}
		else
		{
			// ת��������״̬
			CheckTransition(UOldManWalkingState::StaticClass());
		}
		return;
	}
}