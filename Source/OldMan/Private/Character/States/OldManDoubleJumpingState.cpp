#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManDoubleJumpingState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 消耗二段跳
        Character->hasIntoDoubleJump = true;

        // 应用二段跳速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->DoubleJumpVelocity;
            Jump();
            ResetJumpInput(false);
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInDoubleJump;

        // 播放二段跳动画
        Character->PlayDoubleJumpAnimation();
    }
}

void UOldManDoubleJumpingState::Exit()
{
    Super::Exit();
}

void UOldManDoubleJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovementInAir(DeltaTime);
}

void UOldManDoubleJumpingState::SetupTransitionRules()
{
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);

    // 检查是否开始下落
    ADD_LAMBDA_TRANSITION(UOldManFallingState,
        [this]() {
            return GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0;
        },
        "StartFalling");
}