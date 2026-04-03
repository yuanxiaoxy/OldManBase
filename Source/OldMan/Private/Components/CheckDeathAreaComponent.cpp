// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CheckDeathAreaComponent.h"
#include "Engine/DamageEvents.h"

// Sets default values for this component's properties
UCheckDeathAreaComponent::UCheckDeathAreaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCheckDeathAreaComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCheckDeathAreaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AActor* Owner = GetOwner())
	{
		bool InSafeArea = ADeathSafeAreaManager::GetInstance()->IsLocationSafe(Owner->GetActorLocation());
		if (InSafeArea)
		{
			CurAreaType = EOctreeMode::DefaultSafe;
		}
		else if(!InSafeArea && CurAreaType == EOctreeMode::DefaultSafe)
		{
			CurAreaType = EOctreeMode::DefaultDeath;
			FDamageEvent damageEvent;
			GetOwner()->TakeDamage(100.0f, damageEvent, GetWorld()->GetFirstPlayerController(), GetOwner());
		}
	}
}

