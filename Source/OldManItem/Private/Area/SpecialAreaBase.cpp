// Fill out your copyright notice in the Description page of Project Settings.


#include "Area/SpecialAreaBase.h"
#include "Components/BoxComponent.h"

// Sets default values
ASpecialAreaBase::ASpecialAreaBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    // 创建碰撞组件
    AreaBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AreaBox"));
    AreaBox->SetupAttachment(RootComponent);
    AreaBox->SetCollisionProfileName(TEXT("SpecialArea"));

    // 绑定函数
    AreaBox->OnComponentBeginOverlap.AddDynamic(this, &ASpecialAreaBase::OnOverlayBegin);
    AreaBox->OnComponentEndOverlap.AddDynamic(this, &ASpecialAreaBase::OnOverlayEnd);
}

// Called when the game starts or when spawned
void ASpecialAreaBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpecialAreaBase::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void ASpecialAreaBase::OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

