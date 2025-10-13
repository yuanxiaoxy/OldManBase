#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManDeadState.h"

void UOldManDoubleJumpingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Double Jumping State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// 禁用二段跳
		Character->bCanDoubleJump = false;

		// 应用二段跳速度
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			FVector JumpVelocity = Character->GetCharacterMovement()->Velocity;
			JumpVelocity.Z = Character->CharacterAttributes->DoubleJumpVelocity;
			Character->GetCharacterMovement()->Velocity = JumpVelocity;
		}

		// 播放二段跳动画
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