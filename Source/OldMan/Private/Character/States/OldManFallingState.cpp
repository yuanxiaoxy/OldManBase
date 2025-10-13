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
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManFallingState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	if (!Character->IsFalling() && Character->bJustLanded)
	{
		// ��غ�ת�������״̬
		CheckTransition(UOldManLandState::StaticClass());
		return;
	}
}