#include "Character/States/OldManRunningState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManRunningState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Running State"));

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
    UE_LOG(LogTemp, Log, TEXT("Exiting Running State"));
}

void UOldManRunningState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // 处理移动和旋转
    HandleMovement(DeltaTime);

    // 更新动画
    UpdateAnimation();

    // 检查状态转换
    CheckStateTransitions();
}

void UOldManRunningState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    if (CheckFallingCondition())
    {
        CheckTransition(UOldManFallingState::StaticClass());
        return;
    }

    if (CheckAttackCondition())
    {
        CheckTransition(UOldManAttackingState::StaticClass());
        return;
    }

    if (CheckJumpCondition())
    {
        CheckTransition(UOldManJumpingState::StaticClass());
        return;
    }

    if (!HasMovementInput())
    {
        CheckTransition(UOldManIdleState::StaticClass());
        return;
    }

    if (!IsRunning())
    {
        CheckTransition(UOldManWalkingState::StaticClass());
        return;
    }
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