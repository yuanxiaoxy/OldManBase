#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManDoubleJumpingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Double Jumping State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 禁用二段跳
        Character->hasIntoDoubleJump = true;

        // 应用二段跳速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->JumpZVelocity = Character->CharacterAttributes->DoubleJumpVelocity;
            Jump();
            ResetJumpInput(false);
        }

        // 播放二段跳动画
        Character->PlayDoubleJumpAnimation();
    }
}

void UOldManDoubleJumpingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Double Jumping State"));
}

void UOldManDoubleJumpingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // 在空中也可以移动
    HandleMovement(DeltaTime);

    CheckStateTransitions();
}

void UOldManDoubleJumpingState::CheckStateTransitions()
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
}