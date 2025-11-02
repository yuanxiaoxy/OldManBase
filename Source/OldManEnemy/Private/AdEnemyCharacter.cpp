// AdEnemyCharacter.cpp
#include "AdEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // 添加这一行！
#include "BehaviorTree/BehaviorTree.h"

AAdEnemyCharacter::AAdEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 设置移动速度
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AAdEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = EAdMonsterState::Patrol;
}

void AAdEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAdEnemyCharacter::ChangeState(EAdMonsterState NewState)
{
    EAdMonsterState PreviousState = CurrentState;
    CurrentState = NewState;

    // 这里可以添加状态变化的逻辑，比如播放动画等
    UE_LOG(LogTemp, Warning, TEXT("State changed from %d to %d"), (int32)PreviousState, (int32)NewState);
}

bool AAdEnemyCharacter::PerformConeAttack()
{
    // 实现圆锥检测逻辑
    // 返回是否击中玩家
    return false; // 暂时返回false
}

void AAdEnemyCharacter::PerformLaserAttack()
{
    // 实现激光攻击逻辑
}




void AAdEnemyCharacter::TakeDamage(int32 DamageAmount)
{
    Health -= DamageAmount;
    if (Health <= 0)
    {
        ChangeState(EAdMonsterState::Dead);
        // 处理死亡逻辑
    }
    else
    {
        ChangeState(EAdMonsterState::Hurt);
    }
}