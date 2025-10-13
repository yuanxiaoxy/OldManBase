#include "Character/States/OldManJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManJumpingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Jumping State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// ÆôÓÃ¶þ¶ÎÌø
		Character->bCanDoubleJump = true;

		// ²¥·ÅÌøÔ¾¶¯»­
		Character->PlayJumpAnimation();
	}
}

void UOldManJumpingState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Jumping State"));
}

void UOldManJumpingState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManJumpingState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	if (Character->IsFalling())
	{
		CheckTransition(UOldManFallingState::StaticClass());
		return;
	}
}