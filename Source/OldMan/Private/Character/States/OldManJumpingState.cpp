#include "Character/States/OldManJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManJumpingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Jumping State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // Ӧ����Ծ�ٶ�
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            //������Ծ����
            ResetJumpInput(false);

            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->JumpVelocity;
            Jump();
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInJump;

        // ������Ծ����
        Character->PlayJumpAnimation();
    }
}

void UOldManJumpingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Jumping State"));
}

void UOldManJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // �ڿ���Ҳ�����ƶ�
    HandleMovementInAir(DeltaTime);

    CheckStateTransitions();
}


void UOldManJumpingState::CheckStateTransitions()
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

    // ������������
    if (HasJumpInput() && GetOldManCharacter()->CanDoubleJump())
    {
        CheckTransition(UOldManDoubleJumpingState::StaticClass());
        return;
    }
}