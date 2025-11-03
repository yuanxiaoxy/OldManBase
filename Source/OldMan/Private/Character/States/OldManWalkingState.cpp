#include "Character/States/OldManWalkingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"

void UOldManWalkingState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        targetSpeed = Character->CharacterAttributes->MoveSpeedInWalk;
        // 设置行走速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->MoveSpeedInWalk;
        }
    }
}

void UOldManWalkingState::Exit()
{
    Super::Exit();
}

void UOldManWalkingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
    UpdateAnimation();
}

void UOldManWalkingState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_TRANSITION(UOldManJumpingState, CheckJumpCondition);
    ADD_LAMBDA_TRANSITION(UOldManIdleState,
        [this]() { return !HasMovementInput(); },
        "NoMovementInput");
}

void UOldManWalkingState::UpdateAnimation()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 计算移动速度和方向用于动画混合
        FVector Velocity = Character->GetVelocity();
        FVector Forward = Character->GetActorForwardVector();
        FVector Right = Character->GetActorRightVector();

        float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
        float RightSpeed = FVector::DotProduct(Velocity, Right);

        Character->PlayMoveAnimation(ForwardSpeed, RightSpeed);
    }
}