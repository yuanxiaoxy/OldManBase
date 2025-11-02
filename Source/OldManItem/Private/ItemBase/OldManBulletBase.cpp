// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/OldManBulletBase.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Engine.h"

AOldManBulletBase::AOldManBulletBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(TEXT("AttackItem"));

	MeshComponent->OnComponentHit.AddDynamic(this, &AOldManBulletBase::OnHit);

	// 创建抛射物移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = MeshComponent;
	ProjectileMovement->InitialSpeed = bulletBaseParam.InitialSpeed;
	ProjectileMovement->MaxSpeed = bulletBaseParam.MaxSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	Tags.Add(UGlobalTagName::Tag_DetcedItem);
}

void AOldManBulletBase::BeginPlay()
{
	Super::BeginPlay();
	// 设置生命周期
	SetLifeSpan(bulletBaseParam.LifeSpan);
}

void AOldManBulletBase::Tick(float DeltaTime)     
{
	Super::Tick(DeltaTime);
}

void AOldManBulletBase::InitializeBullet(const FVector& direction, AActor* targetActor)
{
}

void AOldManBulletBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor->Tags.Find(UGlobalTagName::Tag_BeDetcedItem) > -1)
	{
		Attacked(OtherActor);
	}
}
