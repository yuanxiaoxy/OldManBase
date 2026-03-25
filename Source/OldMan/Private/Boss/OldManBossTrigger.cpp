// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/OldManBossTrigger.h"
#include "Boss/OldManHead.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "OldManItem/Public/Bullet/OldManBullet.h"

// Sets default values
AOldManBossTrigger::AOldManBossTrigger()
{
  	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 创建碰撞组件
	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetBoxExtent(FVector(50, 50, 50));
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));

	// 创建视觉组件
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	// 绑定碰撞事件
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AOldManBossTrigger::OnOverlapBegin);
}

// Called when the game starts or when spawned
void AOldManBossTrigger::BeginPlay()
{
	Super::BeginPlay();
	
}

// 碰撞检测回调
void AOldManBossTrigger::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 碰撞检测逻辑已迁移到 TakeDamage 方法
}

// 处理伤害事件
float AOldManBossTrigger::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{

	// 触发 OldManHead 功能
	if (OldManHeadRef)
	{
		OldManHeadRef->Blink(ProgressToAdd);
	}

	// 销毁自身
	//Destroy();

	return DamageAmount;
}

// Called every frame
void AOldManBossTrigger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

