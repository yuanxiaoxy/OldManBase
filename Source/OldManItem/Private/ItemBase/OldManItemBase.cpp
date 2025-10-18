// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/OldManItemBase.h"

const FName AOldManItemBase::DragableItem = "Tag_DragableItem";

// Sets default values
AOldManItemBase::AOldManItemBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AOldManItemBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AOldManItemBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

