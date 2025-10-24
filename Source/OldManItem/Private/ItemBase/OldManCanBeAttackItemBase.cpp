// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/OldManCanBeAttackItemBase.h"

AOldManCanBeAttackItemBase::AOldManCanBeAttackItemBase()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionProfileName(TEXT("CanBeAttackItem"));

	MeshComponent->OnComponentHit.AddDynamic(this, &AOldManCanBeAttackItemBase::OnHit);

	Tags.Add(UGlobalTagName::Tag_DetcedItem);
}

void AOldManCanBeAttackItemBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	BeAttacked();
}
