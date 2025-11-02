#include "Character/States/OldManAttackingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManFallingState.h"

void UOldManAttackingState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        AttackStartTime = GetWorld()->GetTimeSeconds();
        AttackDuration = 0.5f; // 攻击动画持续时间

        // 播放攻击动画
        Character->PlayAttackAnimation();

        // 执行攻击检测
        PerformAttack(Character);

        // 清除攻击输入
        Character->bHasAttackInput = false;
    }
}

void UOldManAttackingState::Exit()
{
    Super::Exit();
}

void UOldManAttackingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
}

void UOldManAttackingState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 攻击动画结束后的转换规则
    ADD_LAMBDA_TRANSITION(UOldManFallingState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return CheckFallingCondition() && (CurrentTime - AttackStartTime >= AttackDuration);
        },
        "AttackEndFalling");

    ADD_LAMBDA_TRANSITION(UOldManIdleState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return !HasMovementInput() && (CurrentTime - AttackStartTime >= AttackDuration);
        },
        "AttackEndIdle");

    ADD_LAMBDA_TRANSITION(UOldManRunningState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return IsRunning() && (CurrentTime - AttackStartTime >= AttackDuration);
        },
        "AttackEndRunning");

    ADD_LAMBDA_TRANSITION(UOldManWalkingState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return HasMovementInput() && !IsRunning() && (CurrentTime - AttackStartTime >= AttackDuration);
        },
        "AttackEndWalking");
}

void UOldManAttackingState::PerformAttack(AOldManCharacter* Character)
{
    if (!Character) return;

    // 执行攻击检测
    Character->PerformAttackDetection();

    UE_LOG(LogTemp, Log, TEXT("Performing attack"));
}