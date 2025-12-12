#include "Character/States/SlopeState/OldManOnSlopeJumpState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/SlopeState/OldManOnSlopeDoubleJumpState.h"
#include "Character/States/SlopeState/OldManOnSlopeFallState.h"

void UOldManOnSlopeJumpState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 应用跳跃速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            //重置跳跃输入
            ResetJumpInput(false);
            // 重置二段跳
            Character->hasIntoDoubleJump = false;

            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->JumpForceInSlope;
            Jump();
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInJumpInSlope;

        // 播放跳跃动画
        Character->PlayJumpAnimation();
    }
}

void UOldManOnSlopeJumpState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeJumpState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 检查二段跳输入
    ADD_LAMBDA_TRANSITION(UOldManOnSlopeDoubleJumpState,
        [this]() {
            return HasJumpInput() && GetOldManCharacter() && GetOldManCharacter()->CanDoubleJump();
        },
        "DoubleJump");

    // 检查是否开始下落
    ADD_LAMBDA_TRANSITION(UOldManOnSlopeFallState,
        [this]() {
            return GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0;
        },
        "StartFalling");
}