// OldManHaloAttack.cpp
// 实现光束攻击的逻辑。

#include "Boss/OldManBeamAttack.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "MonoManager/MonoManager.h"



AOldManBeamAttack::AOldManBeamAttack()
{
    PrimaryActorTick.bCanEverTick = true;

    // 创建碰撞组件并设置为根组件
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    RootComponent = CollisionComponent;
    CollisionComponent->SetSphereRadius(50.0f);
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
    CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AOldManBeamAttack::OnOverlapBegin);

    MoveSpeed = 10.0f;
    Reached = false;
    AddProgress = 1.0f;
    DeleteProgress = 0.0f;
}

void AOldManBeamAttack::BeginPlay()
{
    Super::BeginPlay();
}

void AOldManBeamAttack::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (Reached) return;

    // 检查重叠对象是否是目标老人头
    if (OtherActor && OtherActor == TargetHead)
    {
        Reached = true;
        if (TargetHead)
        {
            // 调用老人头的命中回调，增加进度
            TargetHead->OnAttackHit(AddProgress, ContinueTime);
            UMonoManager* mono = UMonoManager::GetMonoManager(); 
            mono->SetInterval(ContinueTime, "OldManBeamAttackKillMe", this, &AOldManBeamAttack::DestoryMe);
        }
        else DestoryMe();
    }

}

void AOldManBeamAttack::DestoryMe()
{
    UMonoManager* mono = UMonoManager::GetMonoManager();
    mono->ClearTimer("OldManBeamAttackKillMe"); 
    Destroy();
}



void AOldManBeamAttack::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!TargetHead || Reached)
        return;

    // 计算朝向目标的方向
    FVector Direction = (TargetHead->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    if (Direction.IsNearlyZero()) return;

    // 将自身旋转到面向目标的方向（仅绕 Z 轴，保持水平）
    FRotator TargetRotation = Direction.Rotation();
    TargetRotation.Pitch = 0.0f;
    TargetRotation.Roll = 0.0f;
    SetActorRotation(TargetRotation);

    // 向前移动
    FVector NewLocation = GetActorLocation() + GetActorForwardVector() * MoveSpeed * DeltaTime;
    SetActorLocation(NewLocation);
}


