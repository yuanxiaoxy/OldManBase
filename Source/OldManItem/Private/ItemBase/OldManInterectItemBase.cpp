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
    InteractionBox->SetCollisionProfileName(TEXT("InterectItem"));
}

void AOldManInterectItemBase::Interect(FOldManItemInteractData interectData)
{
	UE_LOG(LogTemp, Display, TEXT("Start Interect %s"), *(GetName()));
}