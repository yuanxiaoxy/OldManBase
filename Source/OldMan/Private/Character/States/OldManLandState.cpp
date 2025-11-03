#include "Character/States/OldManLandState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"

void UOldManLandState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        LandStartTime = GetWorld()->GetTimeSeconds();
        LandDuration = Character->CharacterAttributes->LandDuration; // 落地动画持续时间

        // 播放落地动画
        Character->PlayLandAnimation();

        // 重置二段跳
        Character->hasIntoDoubleJump = false;
    }
}

void UOldManLandState::Exit()
{
    Super::Exit();
}

void UOldManLandState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
}

void UOldManLandState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_LAMBDA_TRANSITION(UOldManIdleState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return !HasMovementInput() && (CurrentTime - LandStartTime >= LandDuration);
        },
        "LandEndIdle");

    ADD_LAMBDA_TRANSITION(UOldManRunningState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return IsRunning() && (CurrentTime - LandStartTime >= LandDuration);
        },
        "LandEndRunning");

    ADD_LAMBDA_TRANSITION(UOldManWalkingState,
        [this]() {
            float CurrentTime = GetWorld()->GetTimeSeconds();
            return HasMovementInput() && !IsRunning() && (CurrentTime - LandStartTime >= LandDuration);
        },
        "LandEndWalking");
}