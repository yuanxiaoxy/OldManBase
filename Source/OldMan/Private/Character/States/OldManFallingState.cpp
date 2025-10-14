#include "Character/States/OldManFallingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManFallingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Falling State"));
}

void UOldManFallingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Falling State"));
}

void UOldManFallingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // �ڿ���Ҳ�����ƶ�
    HandleMovement(DeltaTime);

    CheckStateTransitions();
}

void UOldManFallingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // ʹ�ø��ɿ�����ؼ��
    bool bIsActuallyGrounded = Character->IsActuallyGrounded();
    bool bHasJustLanded = Character->bJustLanded;
    float TimeSinceLanding = Character->GetTimeSinceLastLanding();

    UE_LOG(LogTemp, VeryVerbose, TEXT("FallingState Check - IsGrounded: %d, JustLanded: %d, TimeSinceLand: %.3f"),
        bIsActuallyGrounded, bHasJustLanded, TimeSinceLanding);

    // ����Ƿ���أ�ʹ�ø����ɵ�������
    if (!CheckFallingCondition() && (bIsActuallyGrounded || bHasJustLanded || TimeSinceLanding < 0.3f))
    {
        UE_LOG(LogTemp, Log, TEXT("Transitioning from Falling to Land state"));
        CheckTransition(UOldManLandState::StaticClass());
        return;
    }

    // ������������
    if (HasJumpInput() && Character->CanDoubleJump())
    {
        CheckTransition(UOldManDoubleJumpingState::StaticClass());
        return;
    }
}