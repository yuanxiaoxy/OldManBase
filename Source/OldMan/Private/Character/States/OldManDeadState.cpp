#include "Character/States/OldManDeadState.h"
#include "Character/OldManCharacter.h"

void UOldManDeadState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Dead State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		HandleDeath(Character);
	}
}

void UOldManDeadState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Dead State"));
}

void UOldManDeadState::Update(float DeltaTime)
{
	// ����״̬����Ҫ�����߼�
	// ����������������������ĸ��»򸴻��ʱ����
}

void UOldManDeadState::HandleDeath(AOldManCharacter* Character)
{
	// �����ƶ�����ײ
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->DisableMovement();
	}

	// ������������
	Character->PlayDeathAnimation();

	// ���������¼�
	Character->TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("OldManDeath"));

	UE_LOG(LogTemp, Log, TEXT("Character has died"));
}