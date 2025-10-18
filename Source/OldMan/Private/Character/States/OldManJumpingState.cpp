#include "Character/States/OldManJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManJumpingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Jumping State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 应用跳跃速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            //重置跳跃输入
            ResetJumpInput(false);

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
    UE_LOG(LogTemp, Log, TEXT("Exiting Jumping State"));
}

void UOldManJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // 在空中也可以移动
    HandleMovementInAir(DeltaTime);

    CheckStateTransitions();
}


void UOldManJumpingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    // 检查是否开始下落
    if (GetCharacterMovement() && GetCharacterMovement()->Velocity.Z <= 0)
    {
        CheckTransition(UOldManFallingState::StaticClass());
        return;
    }

    // 检查二段跳输入
    if (HasJumpInput() && GetOldManCharacter()->CanDoubleJump())
    {
        CheckTransition(UOldManDoubleJumpingState::StaticClass());
        return;
    }
}