#include "Character/States/OldManFallingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManFallingState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        targetSpeed = Character->CharacterAttributes->MoveSpeedInAir;
    }
}

void UOldManFallingState::Exit()
{
    Super::Exit();
}

void UOldManFallingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovementInAir(DeltaTime);
}

void UOldManFallingState::SetupTransitionRules()
{
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);

    // 检查是否落地
    ADD_LAMBDA_TRANSITION(UOldManLandState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            if (!Character) return false;

            bool bIsActuallyGrounded = Character->IsActuallyGrounded();
            float TimeSinceLanding = Character->GetTimeSinceLastLanding();

            return !CheckFallingCondition() && (bIsActuallyGrounded || TimeSinceLanding < 0.3f);
        },
        "Landing");

    // 检查二段跳输入
    ADD_LAMBDA_TRANSITION(UOldManDoubleJumpingState,
        [this]() {
            return HasJumpInput() && GetOldManCharacter() && GetOldManCharacter()->CanDoubleJump();
        },
        "DoubleJump");
}