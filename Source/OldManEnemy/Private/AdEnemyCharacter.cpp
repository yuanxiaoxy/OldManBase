// AdEnemyCharacter.cpp
#include "AdEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h" // 添加这一行！
#include "BehaviorTree/BehaviorTree.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Character/OldManCharacter.h"
#include "DrawDebugHelpers.h"
#include "AdEnemyAIController.h"
#include "OldManEnemyManager.h"


AAdEnemyCharacter::AAdEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 设置移动速度
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;

}

float AAdEnemyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    ChangeState(EAdMonsterState::Dead);
    OnDespawn_Implementation();
    return 0;
}

void AAdEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    Player = Cast<AOldManCharacter>( UGameplayStatics::GetPlayerCharacter(this, 0));
    CurrentState = EAdMonsterState::Patrol;

   
}

void AAdEnemyCharacter::InitializeEnemy_Implementation(const FEnemyLocationInfo& EnemyInfo)
{
    Path = EnemyInfo.PatrolPath;
}

void AAdEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
#if WITH_EDITOR
    // 仅在编辑器中绘制调试图形
   /* UE_LOG(LogTemp, Log, TEXT("DrawDebugCircle In"));*/
    if (GetWorld() && (GetWorld()->WorldType == EWorldType::Editor ||
        GetWorld()->WorldType == EWorldType::PIE))
    {
        
        FVector ActorLocation = GetActorLocation();
        ActorLocation.Z = 0;
        // 绘制检测半径圆圈（绿色）
        DrawDebugCircle(
            GetWorld(),
            ActorLocation,
            DetectRadius,
            25,                    // 分段数，影响圆形的平滑度
            FColor::Green,
            false,                 // 不持久化（只在当前帧显示）
            -1.0f,                // 持续时间（-1表示每帧都重绘）
            0,                    // 深度优先级
            2.0f,
            FVector(1, 0, 0),  // X轴方向向量
            FVector(0, 1, 0),  // Y轴方向向量
            false              // 不绘制坐标轴// 线条粗细
        );

        // 绘制攻击半径圆圈（红色）
        DrawDebugCircle(
            GetWorld(),
            ActorLocation,
            AttackRadius,
            25,
            FColor::Red,
            false,
            -1.0f,
            0,
            2.0f,
            FVector(1, 0, 0),  // X轴方向向量
            FVector(0, 1, 0),  // Y轴方向向量
            false              // 不绘制坐标轴
        );

        // 可选：在圆心位置绘制一个点，更易识别中心
        DrawDebugPoint(
            GetWorld(),
            ActorLocation,
            10.0f,                // 点的大小
            FColor::Blue,
            false,
            -1.0f
        );
        //UE_LOG(LogTemp, Log,TEXT("DrawDebugCircle - 调试图形正在绘制, 检测半径 %f, 攻击半径 %f."), DetectRadius, AttackRadius);
    }
#endif
}

void AAdEnemyCharacter::ChangeState(EAdMonsterState NewState)
{
    EAdMonsterState PreviousState = CurrentState;
    CurrentState = NewState;

    // 这里可以添加状态变化的逻辑，比如播放动画等
    //UE_LOG(LogTemp, Warning, TEXT("State changed from %d to %d"), (int32)PreviousState, (int32)NewState);
}

bool AAdEnemyCharacter::DectectPlayer()
{
        // 2. 添加更严格的空指针检查
        if (!Player)
        {
            UE_LOG(LogTemp,Error, TEXT("EnemyCharacter,Player == null!"));
            return false;
        }

        // 3. 计算XY平面距离
        float DistanceToPlayer = FVector::DistXY(GetActorLocation(),
            Player->GetActorLocation());

        // 4. 距离检测
        if (DistanceToPlayer <= DetectRadius)
        {
            //UE_LOG(LogTemp, Warning, TEXT("FindPlayer!"));
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
        UE_LOG(LogTemp, Log, TEXT("AttackPlayer: %d"), AttackPower);
    }
}




void AAdEnemyCharacter::TakeDamage(int32 DamageAmount)
{
    CurrentHealth -= DamageAmount;
    if (Health <= 0)
    {
        ChangeState(EAdMonsterState::Dead);
        OnDespawn_Implementation();
    }
    else
    {
        /*ChangeState(EAdMonsterState::Hurt);*/
    }
}




void AAdEnemyCharacter::OnSpawn_Implementation()
{
    // 被对象池取出时调用：重置角色状态
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(true);
    CurrentHealth = Health; // 重置生命值
    bIsDead = false;
    // 确保 Controller 正确设置 - 原始指针版本
    if (!AIController) // 直接检查指针是否为nullptr
    {
        AIController = Cast<AAdEnemyAIController>(GetController());   
    }
    AIController->OnEnemySpawn();
    AIController->EnemyCharacter = this;
}

void AAdEnemyCharacter::OnDespawn_Implementation()
{
    // 被回收到对象池时调用：清理角色状态
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false); 
    if (bIsDead) return;
    bIsDead = true;
    // ... 角色自身的死亡逻辑
    if (AIController) {
        AIController->OnEnemyDeath(); // 通知控制器处理死亡
    }
}