#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManDoubleJumpingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Double Jumping State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// ���ö�����
		Character->bCanDoubleJump = false;

		// Ӧ�ö������ٶ�
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			FVector JumpVelocity = Character->GetCharacterMovement()->Velocity;
			JumpVelocity.Z = Character->CharacterAttributes->DoubleJumpVelocity;
			Character->GetCharacterMovement()->Velocity = JumpVelocity;
		}

		// ���Ŷ���������
		Character->PlayDoubleJumpAnimation();
	}
}

void UOldManDoubleJumpingState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Double Jumping State"));
}

void UOldManDoubleJumpingState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManDoubleJumpingState::CheckStateTransitions(AOldManCharacter* Character)
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