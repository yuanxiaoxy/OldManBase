#include "Character/States/OldManAttackingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManAttackingState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Attacking State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        AttackStartTime = GetWorld()->GetTimeSeconds();
        AttackDuration = 0.5f; // ¹¥»÷¶¯»­³ÖÐøÊ±¼ä

        // ²¥·Å¹¥»÷¶¯»­
        Character->PlayAttackAnimation();

        // Ö´ÐÐ¹¥»÷¼ì²â
        PerformAttack(Character);

        // Çå³ý¹¥»÷ÊäÈë
        Character->bHasAttackInput = false;
    }
}

void UOldManAttackingState::Exit()
{
    UE_LOG(LogTemp, Log, TEXT("Exiting Attacking State"));
}

void UOldManAttackingState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    CheckStateTransitions();
}

void UOldManAttackingState::CheckStateTransitions()
{
    if (CheckDeathCondition())
    {
        CheckTransition(UOldManDeadState::StaticClass());
        return;
    }

    // ¹¥»÷¶¯»­½áÊøºó¸ù¾ÝÒÆ¶¯×´Ì¬×ª»»
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - AttackStartTime >= AttackDuration)
    {
        if (CheckFallingCondition())
        {
            CheckTransition(UOldManFallingState::StaticClass());
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

void UOldManAttackingState::PerformAttack(AOldManCharacter* Character)
{
    if (!Character) return;

    // Ö´ÐÐ¹¥»÷¼ì²â
    Character->PerformAttackDetection();

    UE_LOG(LogTemp, Log, TEXT("Performing attack"));
}