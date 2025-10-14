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
    UE_LOG(LogTemp, Log, TEXT("Entering Idle State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 重置移动速度到行走速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->WalkSpeed;
        }

        // 调用蓝图动画事件
        Character->PlayMoveAnimation(0.0f, 0.0f);
    }
}

void UOldManIdleState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Idle State"));
}

void UOldManIdleState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    CheckStateTransitions();
}

void UOldManIdleState::CheckStateTransitions()
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

    if (HasMovementInput())
    {
        if (IsRunning())
        {
            CheckTransition(UOldManRunningState::StaticClass());
        }
        else
        {
            CheckTransition(UOldManWalkingState::StaticClass());
        }
    }
}