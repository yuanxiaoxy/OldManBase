#include "Character/States/OldManIdleState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"

void UOldManIdleState::Enter()
{
	// 站立状态进入逻辑
	UE_LOG(LogTemp, Log, TEXT("Entering Idle State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		// 重置移动速度到行走速度
		if (Character->GetCharacterMovement() && Character->CharacterAttributes)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->WalkSpeed;
		}

		// 调用蓝图动画事件
		Character->PlayMoveAnimation(0.0f, 0.0f);
	}
}

void UOldManIdleState::Exit()
{
	// 站立状态退出逻辑
	UE_LOG(LogTemp, Log, TEXT("Exiting Idle State"));
}

void UOldManIdleState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManIdleState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		// 转换到死亡状态
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	if (Character->IsFalling())
	{
		// 转换到下落状态
		CheckTransition(UOldManFallingState::StaticClass());
		return;
	}

	if (Character->IsMoving())
	{
		if (Character->bIsRunning)
		{
			// 转换到跑步状态
			CheckTransition(UOldManRunningState::StaticClass());
		}
		else
		{
			// 转换到行走状态
			CheckTransition(UOldManWalkingState::StaticClass());
		}
		return;
	}
}