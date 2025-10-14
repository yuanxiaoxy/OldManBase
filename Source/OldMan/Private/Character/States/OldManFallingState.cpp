#include "Character/States/OldManFallingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManFallingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Falling State"));
}

void UOldManFallingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Falling State"));
}

void UOldManFallingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // 在空中也可以移动
    HandleMovement(DeltaTime);

    CheckStateTransitions();
}

void UOldManFallingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // 使用更可靠的落地检测
    bool bIsActuallyGrounded = Character->IsActuallyGrounded();
    bool bHasJustLanded = Character->bJustLanded;
    float TimeSinceLanding = Character->GetTimeSinceLastLanding();

    UE_LOG(LogTemp, VeryVerbose, TEXT("FallingState Check - IsGrounded: %d, JustLanded: %d, TimeSinceLand: %.3f"),
        bIsActuallyGrounded, bHasJustLanded, TimeSinceLanding);

    // 检查是否落地（使用更宽松的条件）
    if (!CheckFallingCondition() && (bIsActuallyGrounded || bHasJustLanded || TimeSinceLanding < 0.3f))
    {
        UE_LOG(LogTemp, Log, TEXT("Transitioning from Falling to Land state"));
        CheckTransition(UOldManLandState::StaticClass());
        return;
    }

    // 检查二段跳输入
    if (HasJumpInput() && Character->CanDoubleJump())
    {
        CheckTransition(UOldManDoubleJumpingState::StaticClass());
        return;
    }
}