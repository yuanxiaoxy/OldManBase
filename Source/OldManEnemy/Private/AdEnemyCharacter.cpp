// AdEnemyCharacter.cpp
#include "AdEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // 添加这一行！
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Character/OldManCharacter.h"


AAdEnemyCharacter::AAdEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 设置移动速度
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    Player = Cast<AOldManCharacter>( UGameplayStatics::GetPlayerCharacter(this, 0));

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

bool AAdEnemyCharacter::DectectPlayer()
{
        // 2. 添加更严格的空指针检查
        if (!Player)
        {
            return false;
        }

        // 3. 计算XY平面距离
        float DistanceToPlayer = FVector::DistXY(GetActorLocation(),
            Player->GetActorLocation());

        // 4. 距离检测
        if (DistanceToPlayer <= DetectRadius)
        {
            return true;
        }

        return false;
    
}

void AAdEnemyCharacter::PerformLaserAttack()
{
    if (!Player) return;

    float DistanceToPlayer = FVector::DistXY(GetActorLocation(), Player->GetActorLocation());
    if (DistanceToPlayer <= AttackRadius)
    {     
        FDamageEvent DamageEvent;
        Player->TakeDamage(AttackPower, DamageEvent, GetController(), this);
    }
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
        /*ChangeState(EAdMonsterState::Hurt);*/
    }
}