#include "Character/States/OldManLandState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManAttackingState.h"

void UOldManLandState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Land State"));

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
    UE_LOG(LogTemp, Log, TEXT("Exiting Land State"));
}

void UOldManLandState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    CheckStateTransitions();
}

void UOldManLandState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    // 落地动画结束后根据移动状态转换
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LandStartTime >= LandDuration)
    {
        if (CheckAttackCondition())
        {
            CheckTransition(UOldManAttackingState::StaticClass());
        }
        else if (!HasMovementInput())
        {
            CheckTransition(UOldManIdleState::StaticClass());
        }
        else if (IsRunning())
        {
            CheckTransition(UOldManRunningState::StaticClass());
        }
        else
        {
            CheckTransition(UOldManWalkingState::StaticClass());
        }
    }
}