#include "Character/States/SlopeState/OldManOnSlopeFallState.h"
#include "Character/States/SlopeState/OldManOnSlopeLandState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/SlopeState/OldManOnSlopeDoubleJumpState.h"

void UOldManOnSlopeFallState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        targetSpeed = Character->CharacterAttributes->MoveSpeedInAirInSlope;
    }
}

void UOldManOnSlopeFallState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeFallState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 检查是否落地
    ADD_LAMBDA_TRANSITION(UOldManOnSlopeLandState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            if (!Character) return false;

            bool bIsActuallyGrounded = Character->IsActuallyGrounded();
            float TimeSinceLanding = Character->GetTimeSinceLastLanding();

            return !CheckFallingCondition() && (bIsActuallyGrounded || TimeSinceLanding < 0.3f);
        },
        "Landing");

    // 检查二段跳输入
    ADD_LAMBDA_TRANSITION(UOldManOnSlopeDoubleJumpState,
        [this]() {
            return HasJumpInput() && GetOldManCharacter() && GetOldManCharacter()->CanDoubleJump();
        },
        "DoubleJump");
}