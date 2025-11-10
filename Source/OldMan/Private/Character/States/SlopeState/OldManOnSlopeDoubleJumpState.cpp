#include "Character/States/SlopeState/OldManOnSlopeDoubleJumpState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/SlopeState/OldManOnSlopeFallState.h"
#include "Character/States/OldManDeadState.h"

void UOldManOnSlopeDoubleJumpState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 消耗二段跳
        Character->hasIntoDoubleJump = true;

        // 应用二段跳速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->MoveSpeedInDoubleJumpInSlope;
            Jump();
            ResetJumpInput(false);
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInDoubleJumpInSlope;

        // 播放二段跳动画
        Character->PlayDoubleJumpAnimation();
    }
}

void UOldManOnSlopeDoubleJumpState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeDoubleJumpState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 检查是否开始下落
    ADD_LAMBDA_TRANSITION(UOldManOnSlopeFallState,
        [this]() {
            return GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0;
        },
        "StartFalling");
}
