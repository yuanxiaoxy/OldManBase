// Fill out your copyright notice in the Description page of Project Settings.


#include "PhoneNumberActor/PhoneNumberManager.h"

// Sets default values
APhoneNumberManager::APhoneNumberManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void APhoneNumberManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitAllSecretGroup();
}

void APhoneNumberManager::InitSecretGroupByGroupName(FString groupName)
{
	if (PhoneNumberGroup.Find(groupName))
	{
		PhoneNumberGroup[groupName].CurSecret = "";
		for (int i = 0; i < PhoneNumberGroup[groupName].PhoneNumbers.Num(); i++)
		{
			auto phoneNumber = PhoneNumberGroup[groupName].PhoneNumbers[i];
			phoneNumber->InitPhoneNumberActor(i, groupName, this);
		}

		// 为每个组创建线段生成器
		PhoneNumberGroup[groupName].LineGenerator = NewObject<ULineGenerator>();
		PhoneNumberGroup[groupName].LineGenerator->InitializeSplineComponents(
			this,
			PhoneNumberGroup[groupName].LineStaticMesh,
			PhoneNumberGroup[groupName].LineForwardAxis,
			PhoneNumberGroup[groupName].LineWidth,
			PhoneNumberGroup[groupName].LineMaterial
		);
	}

	if (LastActivatedPhoneNumber.Find(groupName))
	{
		LastActivatedPhoneNumber[groupName] = nullptr;
	}
}

void APhoneNumberManager::InitAllSecretGroup()
{
	for (auto group : PhoneNumberGroup)
	{
		InitSecretGroupByGroupName(group.Key);
	}

	LastActivatedPhoneNumber.Empty();
}

void APhoneNumberManager::ResetSecretGroupByName(FString groupName)
{
	if (PhoneNumberGroup.Find(groupName))
	{
		PhoneNumberGroup[groupName].CurSecret = "";
		for (int i = 0; i < PhoneNumberGroup[groupName].PhoneNumbers.Num(); i++)
		{
			auto phoneNumber = PhoneNumberGroup[groupName].PhoneNumbers[i];
			phoneNumber->Reset();
		}
	}
}

void APhoneNumberManager::ResetAllSecretGroup()
{
	for (auto group : PhoneNumberGroup)
	{
		ResetSecretGroupByName(group.Key);
	}
}

void APhoneNumberManager::EnablePhoneNumberByGroupName(FString name, int number)
{
	if (PhoneNumberGroup.Find(name))
	{
		PhoneNumberGroup[name].CurSecret += FString::FromInt(number);
		if (PhoneNumberGroup[name].CurSecret.Equals(PhoneNumberGroup[name].TargetSecret))
		{
			PhoneNumberGroup[name].PhoneNumberTriggerActor->BeTriggered(true);
		}
		else if (PhoneNumberGroup[name].CurSecret.Len() == PhoneNumberGroup[name].TargetSecret.Len() &&
			!PhoneNumberGroup[name].CurSecret.Equals(PhoneNumberGroup[name].TargetSecret))
		{
			ResetSecretGroupByName(name);
		}
	}
}
