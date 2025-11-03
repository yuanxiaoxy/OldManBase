#include "Character/States/OldManIdleState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"

void UOldManIdleState::Enter()
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

void UOldManIdleState::Exit()
{
    Super::Exit();
}

void UOldManIdleState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
}

void UOldManIdleState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 添加Idle状态特有的转换规则
    ADD_TRANSITION(UOldManJumpingState, CheckJumpCondition);
    ADD_LAMBDA_TRANSITION(UOldManWalkingState,
        [this]() { return HasMovementInput(); },
        "MovementInput");
}