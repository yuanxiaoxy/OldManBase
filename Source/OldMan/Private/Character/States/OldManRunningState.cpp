#include "Character/States/OldManRunningState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManRunningState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Running State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// 设置跑步速度
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->RunSpeed;
		}
	}
}

void UOldManRunningState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Running State"));
}

void UOldManRunningState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
		UpdateAnimation(Character);
	}
}

void UOldManRunningState::CheckStateTransitions(AOldManCharacter* Character)
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

	if (!Character->IsMoving())
	{
		CheckTransition(UOldManIdleState::StaticClass());
		return;
	}

	if (!Character->bIsRunning)
	{
		CheckTransition(UOldManWalkingState::StaticClass());
		return;
	}
}

void UOldManRunningState::UpdateAnimation(AOldManCharacter* Character)
{
	FVector Velocity = Character->GetVelocity();
	FVector Forward = Character->GetActorForwardVector();
	FVector Right = Character->GetActorRightVector();

	float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
	float RightSpeed = FVector::DotProduct(Velocity, Right);

	// 跑步状态传递更高的速度值
	Character->PlayMoveAnimation(ForwardSpeed * 1.5f, RightSpeed * 1.5f);
}