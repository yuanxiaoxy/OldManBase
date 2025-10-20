// Fill out your copyright notice in the Description page of Project Settings.

#include "ItemBase/OldManInterectItemBase.h"
#include "Components/BoxComponent.h"

AOldManInterectItemBase::AOldManInterectItemBase()
{
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    // 创建碰撞组件
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("PlayerInterectItem"));

    // 绑定函数
    InteractionBox->OnComponentBeginOverlap.AddDynamic(this, &AOldManInterectItemBase::OnOverlayBegin);
    InteractionBox->OnComponentEndOverlap.AddDynamic(this, &AOldManInterectItemBase::OnOverlayEnd);
}

void AOldManInterectItemBase::Interect(FOldManItemInteractData interectData)
{
	UE_LOG(LogTemp, Display, TEXT("Start Interect %s"), *(GetName()));
}

void AOldManInterectItemBase::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    OnEnterTrigger(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AOldManInterectItemBase::OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    OnOverlayEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}