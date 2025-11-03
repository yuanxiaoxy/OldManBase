#include "Character/States/OldManDeadState.h"
#include "Character/OldManCharacter.h"

void UOldManDeadState::Enter()
{
    UE_LOG(LogTemp, Log, TEXT("Entering Dead State"));

    if (AOldManCharacter* Character = GetOldManCharacter())
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
}

void UOldManDeadState::HandleDeath(AOldManCharacter* Character)
{
    if (!Character) return;

    // 禁用移动
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
    }

    // 清除所有输入
    Character->MovementInputVector = FVector::ZeroVector;
    Character->bHasJumpInput = false;
    Character->bHasAttackInput = false;
    Character->bIsRunning = false;

    // 播放死亡动画
    Character->PlayDeathAnimation();

    // 触发死亡事件
    Character->TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("OldManDeath"));

    UE_LOG(LogTemp, Log, TEXT("Character has died"));
}