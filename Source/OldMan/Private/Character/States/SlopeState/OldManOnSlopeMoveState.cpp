#include "Character/States/SlopeState/OldManOnSlopeMoveState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/SlopeState/OldManOnSlopeFallState.h"
#include "Character/States/SlopeState/OldManOnSlopeJumpState.h"

void UOldManOnSlopeMoveState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 重置移动速度为行走速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->MoveSpeedInSlope *
                Character->CharacterAttributes->MoveSpeedMultiInSlope;
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedInSlope;

        // 调用移动动画事件
        Character->PlayOnSlopeMoveAnimation();
    }
}

void UOldManOnSlopeMoveState::SetupTransitionRules()
{
	Super::SetupTransitionRules();

    ADD_TRANSITION(UOldManOnSlopeJumpState, CheckJumpCondition);
	ADD_TRANSITION(UOldManOnSlopeFallState, CheckFallingCondition);
}
