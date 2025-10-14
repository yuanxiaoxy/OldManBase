#include "Character/States/OldManWalkingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManWalkingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Walking State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // ���������ٶ�
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->WalkSpeed;
        }
    }
}

void UOldManWalkingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Walking State"));
}

void UOldManWalkingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // �����ƶ�����ת
    HandleMovement(DeltaTime);

    // ���¶���
    UpdateAnimation();

    // ���״̬ת��
    CheckStateTransitions();
}

void UOldManWalkingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    if (CheckFallingCondition())
    {
        CheckTransition(UOldManFallingState::StaticClass());
        return;
    }

    if (CheckAttackCondition())
    {
        CheckTransition(UOldManAttackingState::StaticClass());
        return;
    }

    if (CheckJumpCondition())
    {
        CheckTransition(UOldManJumpingState::StaticClass());
        return;
    }

    if (!HasMovementInput())
    {
        CheckTransition(UOldManIdleState::StaticClass());
        return;
    }

    if (IsRunning())
    {
        CheckTransition(UOldManRunningState::StaticClass());
        return;
    }
}

void UOldManWalkingState::UpdateAnimation()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // �����ƶ��ٶȺͷ������ڶ������
        FVector Velocity = Character->GetVelocity();
        FVector Forward = Character->GetActorForwardVector();
        FVector Right = Character->GetActorRightVector();

        float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
        float RightSpeed = FVector::DotProduct(Velocity, Right);

        Character->PlayMoveAnimation(ForwardSpeed, RightSpeed);
    }
}