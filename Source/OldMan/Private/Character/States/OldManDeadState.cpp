#include "Character/States/OldManDeadState.h"
#include "Character/OldManCharacter.h"

void UOldManDeadState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Dead State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		HandleDeath(Character);
	}
}

void UOldManDeadState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Dead State"));
}

void UOldManDeadState::Update(float DeltaTime)
{
	// 死亡状态不需要更新逻辑
	// 可以在这里添加死亡动画的更新或复活计时器等
}

void UOldManDeadState::HandleDeath(AOldManCharacter* Character)
{
	// 禁用移动和碰撞
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->DisableMovement();
	}

	// 播放死亡动画
	Character->PlayDeathAnimation();

	// 触发死亡事件
	Character->TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("OldManDeath"));

	UE_LOG(LogTemp, Log, TEXT("Character has died"));
}