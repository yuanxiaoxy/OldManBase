#include "Character/States/OldManDeadState.h"
#include "Character/OldManCharacter.h"

void UOldManDeadState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Dead State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
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
}

void UOldManDeadState::HandleDeath(AOldManCharacter* Character)
{
    if (!Character) return;

    // �����ƶ�
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // �����������
    Character->MovementInputVector = FVector::ZeroVector;
    Character->bHasJumpInput = false;
    Character->bHasAttackInput = false;
    Character->bIsRunning = false;

    // ������������
    Character->PlayDeathAnimation();

    // ���������¼�
    Character->TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("OldManDeath"));

    UE_LOG(LogTemp, Log, TEXT("Character has died"));
}