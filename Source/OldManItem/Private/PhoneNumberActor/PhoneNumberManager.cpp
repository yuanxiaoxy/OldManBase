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
		LastActivatedPhoneNumber[groupName].LastActivatedPhoneNumber = nullptr;
		LastActivatedPhoneNumber[groupName].lastIndex = 0;
	}
	else
	{
		LastActivatedPhoneNumber.Add(groupName, FGenerateLineData());
	}
}

void APhoneNumberManager::InitAllSecretGroup()
{
	for (auto group : PhoneNumberGroup)
	{
		InitSecretGroupByGroupName(group.Key);
	}
}

void APhoneNumberManager::ResetSecretGroupByName(FString groupName)
{
	if (PhoneNumberGroup.Find(groupName))
	{
		PhoneNumberGroup[groupName].LineGenerator->ClearAllLines();
		LastActivatedPhoneNumber[groupName].LastActivatedPhoneNumber = nullptr;
		LastActivatedPhoneNumber[groupName].lastIndex = 0;

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

void APhoneNumberManager::EnablePhoneNumberByGroupName(FString name, int number, APhoneNumberActor* CurActivatedPhoneNumber)
{
	if (PhoneNumberGroup.Find(name))
	{
		PhoneNumberGroup[name].CurSecret += FString::FromInt(number);
		GenerateLineBetweenActors(name, LastActivatedPhoneNumber[name], CurActivatedPhoneNumber);

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

void APhoneNumberManager::GenerateLineBetweenActors(FString GroupName, FGenerateLineData& lineData, APhoneNumberActor* ToActor)
{
	if (!PhoneNumberGroup.Contains(GroupName) || !lineData.LastActivatedPhoneNumber || !ToActor)
	{
		lineData.LastActivatedPhoneNumber = ToActor;
		return;
	}

	FPhoneNumberData& Group = PhoneNumberGroup[GroupName];
	if (!Group.LineGenerator)
	{
		return;
	}

	// 生成线段
	Group.LineGenerator->GenerateLine(
		lineData.LastActivatedPhoneNumber->GetActorLocation() - GetActorLocation(),
		ToActor->GetActorLocation() - GetActorLocation(),
		lineData.lastIndex
	);

	lineData.LastActivatedPhoneNumber = ToActor;
	lineData.lastIndex += 1;

	UE_LOG(LogTemp, Warning, TEXT("Generated line between %s and %s in group %s"),
		*lineData.LastActivatedPhoneNumber->GetName(), *ToActor->GetName(), *GroupName);
}
