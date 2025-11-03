#include "Character/States/OldManRunningState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"

void UOldManRunningState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 设置跑步速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->MoveSpeedInWalk;
        }
    }
}

void UOldManRunningState::Exit()
{
    Super::Exit();
}

void UOldManRunningState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
    UpdateAnimation();
}

void UOldManRunningState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_TRANSITION(UOldManJumpingState, CheckJumpCondition);
    ADD_LAMBDA_TRANSITION(UOldManIdleState,
        [this]() { return !HasMovementInput(); },
        "NoMovementInput");
    ADD_LAMBDA_TRANSITION(UOldManWalkingState,
        [this]() { return !IsRunning(); },
        "StopRunning");
}

void UOldManRunningState::UpdateAnimation()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        FVector Velocity = Character->GetVelocity();
        FVector Forward = Character->GetActorForwardVector();
        FVector Right = Character->GetActorRightVector();

        float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
        float RightSpeed = FVector::DotProduct(Velocity, Right);

        // 跑步状态使用更快的动画混合
        Character->PlayMoveAnimation(ForwardSpeed * 1.5f, RightSpeed * 1.5f);
    }
}