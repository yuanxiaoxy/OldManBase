#include "Character/States/OldManAttackingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManDeadState.h"

void UOldManAttackingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Attacking State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		AttackStartTime = GetWorld()->GetTimeSeconds();
		AttackDuration = 0.5f; // ������������ʱ��

		// ���Ź�������
		Character->PlayAttackAnimation();

		// ִ�й����߼�
		PerformAttack(Character);
	}
}

void UOldManAttackingState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Attacking State"));
}

void UOldManAttackingState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManAttackingState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	// ��������������ƶ�״̬ת��
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - AttackStartTime >= AttackDuration)
	{
		if (!Character->IsMoving())
		{
			CheckTransition(UOldManIdleState::StaticClass());
		}
		else if (Character->bIsRunning)
		{
			CheckTransition(UOldManRunningState::StaticClass());
		}
		else
		{
			CheckTransition(UOldManWalkingState::StaticClass());
		}
	}
}

void UOldManAttackingState::PerformAttack(AOldManCharacter* Character)
{
	if (!Character->CharacterAttributes) return;

	// �򵥵Ĺ������
	FVector StartLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector EndLocation = StartLocation + ForwardVector * Character->CharacterAttributes->AttackRange;

	// ���������Ӹ����ӵĹ�������߼�
	// �������߼�⡢�������

	UE_LOG(LogTemp, Log, TEXT("Performing attack with damage: %.1f"), Character->CharacterAttributes->AttackDamage);
}