#include "Character/States/OldManWalkingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManWalkingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Walking State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// 设置行走速度
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->WalkSpeed;
		}
	}
}

void UOldManWalkingState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Walking State"));
}

void UOldManWalkingState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
		UpdateAnimation(Character);
	}
}

void UOldManWalkingState::CheckStateTransitions(AOldManCharacter* Character)
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

	if (Character->bIsRunning)
	{
		CheckTransition(UOldManRunningState::StaticClass());
		return;
	}
}

void UOldManWalkingState::UpdateAnimation(AOldManCharacter* Character)
{
	// 计算移动速度和方向用于动画混合
	FVector Velocity = Character->GetVelocity();
	FVector Forward = Character->GetActorForwardVector();
	FVector Right = Character->GetActorRightVector();

	float ForwardSpeed = FVector::DotProduct(Velocity, Forward);
	float RightSpeed = FVector::DotProduct(Velocity, Right);

	Character->PlayMoveAnimation(ForwardSpeed, RightSpeed);
}