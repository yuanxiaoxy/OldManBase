#include "Character/States/SlopeState/OldManFadeInSlopeState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/SlopeState/OldManOnSlopeMoveState.h"
#include "MonoManager/MonoManager.h"

void UOldManFadeInSlopeState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        Character->SetUseCustomGravity(true);
        Character->PlayFadeInSlopeAnimation();
    }

    UMonoManager::GetMonoManager()->SetTimeout<UOldManFadeInSlopeState>(CachedOldManCharacter->CharacterAttributes->FadeInSlopeStateTime, this, &UOldManFadeInSlopeState::CheckToMoveState);
}

void UOldManFadeInSlopeState::Exit()
{
    Super::Exit();
}

void UOldManFadeInSlopeState::CheckToMoveState()
{
    CheckTransition(UOldManOnSlopeMoveState::StaticClass());
}