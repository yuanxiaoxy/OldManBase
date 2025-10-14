#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManDoubleJumpingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Double Jumping State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // ���ö�����
        Character->hasIntoDoubleJump = true;

        // Ӧ�ö������ٶ�
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->DoubleJumpVelocity;
            Jump();
            ResetJumpInput(false);
        }

        // ���Ŷ���������
        Character->PlayDoubleJumpAnimation();
    }
}

void UOldManDoubleJumpingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Double Jumping State"));
}

void UOldManDoubleJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // �ڿ���Ҳ�����ƶ�
    HandleMovement(DeltaTime);

    CheckStateTransitions();
}

void UOldManDoubleJumpingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    // ����Ƿ�ʼ����
    if (GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0)
    {
        CheckTransition(UOldManFallingState::StaticClass());
        return;
    }
}