#include "Character/States/OldManAttackingState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManDeadState.h"

void UOldManAttackingState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Attacking State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		AttackStartTime = GetWorld()->GetTimeSeconds();
		AttackDuration = 0.5f; // 攻击动画持续时间

		// 播放攻击动画
		Character->PlayAttackAnimation();

		// 执行攻击逻辑
		PerformAttack(Character);
	}
}

void UOldManAttackingState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Attacking State"));
}

void UOldManAttackingState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManAttackingState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	// 攻击结束后根据移动状态转换
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - AttackStartTime >= AttackDuration)
	{
		if (!Character->IsMoving())
		{
			CheckTransition(UOldManIdleState::StaticClass());
		}
		else if (Character->bIsRunning)
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
	if (!Character->CharacterAttributes) return;

	// 简单的攻击检测
	FVector StartLocation = Character->GetActorLocation();
	FVector ForwardVector = Character->GetActorForwardVector();
	FVector EndLocation = StartLocation + ForwardVector * Character->CharacterAttributes->AttackRange;

	// 这里可以添加更复杂的攻击检测逻辑
	// 比如射线检测、球体检测等

	UE_LOG(LogTemp, Log, TEXT("Performing attack with damage: %.1f"), Character->CharacterAttributes->AttackDamage);
}