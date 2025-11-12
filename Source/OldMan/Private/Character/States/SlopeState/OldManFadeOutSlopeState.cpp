#include "Character/States/SlopeState/OldManFadeOutSlopeState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "MonoManager/MonoManager.h"

void UOldManFadeOutSlopeState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        Character->SetUseCustomGravity(false);
        Character->PlayFadeOutSlopeAnimation();
    }

    UMonoManager::GetMonoManager()->SetTimeout<UOldManFadeOutSlopeState>(CachedOldManCharacter->CharacterAttributes->FadeOutSlopeStateTime, this, &UOldManFadeOutSlopeState::CheckToMoveState);
}

void UOldManFadeOutSlopeState::Exit()
{
    Super::Exit();
}

void UOldManFadeOutSlopeState::CheckToMoveState()
{
    CheckTransition(UOldManIdleState::StaticClass());
}