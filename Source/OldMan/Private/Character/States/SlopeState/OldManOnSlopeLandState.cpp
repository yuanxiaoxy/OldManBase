#include "Character/States/SlopeState/OldManOnSlopeLandState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/SlopeState/OldManOnSlopeMoveState.h"

void UOldManOnSlopeLandState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 播放落地动画
        Character->PlayLandAnimation();

        // 重置二段跳
        Character->hasIntoDoubleJump = false;
    }
}

void UOldManOnSlopeLandState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeLandState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_LAMBDA_TRANSITION(UOldManIdleState,
        [this]() {
            return !CheckOnSlopeCondition();
        },
        "LandEndIdle");

    ADD_LAMBDA_TRANSITION(UOldManOnSlopeMoveState,
        [this]() {
            return CheckOnSlopeCondition();
        },
        "LandEndToSlope");
}