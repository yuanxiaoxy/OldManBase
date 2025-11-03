#include "Character/States/OldManJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManJumpingState::Enter()
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

            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->JumpVelocity;
            Jump();
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInJump;

        // 播放跳跃动画
        Character->PlayJumpAnimation();
    }
}

void UOldManJumpingState::Exit()
{
    Super::Exit();
}

void UOldManJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovementInAir(DeltaTime);
}

void UOldManJumpingState::SetupTransitionRules()
{
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);

    // 检查是否开始下落
    ADD_LAMBDA_TRANSITION(UOldManFallingState,
        [this]() {
            return GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0;
        },
        "StartFalling");

    // 检查二段跳输入
    ADD_LAMBDA_TRANSITION(UOldManDoubleJumpingState,
        [this]() {
            return HasJumpInput() && GetOldManCharacter() && GetOldManCharacter()->CanDoubleJump();
        },
        "DoubleJump");
}