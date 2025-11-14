#include "Character/States/CableState/OldManOnCableHorizontalJumpState.h"
#include "Character/OldManCharacter.h"

void UOldManOnCableHorizontalJumpState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 重置移动速度为行走速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->MoveSpeedInWalk;
        }

        targetSpeed = 0.0f;

        // 调用移动动画事件
        Character->PlayMoveAnimation(0.0f, 0.0f);
    }
}

void UOldManOnCableHorizontalJumpState::Exit()
{
    Super::Exit();
}

void UOldManOnCableHorizontalJumpState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
}

void UOldManOnCableHorizontalJumpState::SetupTransitionRules()
{
    Super::SetupTransitionRules();
}