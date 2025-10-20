// Fill out your copyright notice in the Description page of Project Settings.


#include "PhoneNumberActor/PhoneNumberActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GlobalTagName.h"
#include "PhoneNumberActor/PhoneNumberManager.h"

APhoneNumberActor::APhoneNumberActor()
{

}

void APhoneNumberActor::InitPhoneNumberActor(int number, FString groupName, APhoneNumberManager* PhoneNumberManager)
{
	inEnableState = false;
	thisNumber = number;
	thisGroup = groupName;
	phoneNumberManager = PhoneNumberManager;
}

void APhoneNumberActor::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Tags.Find(UGlobalTagName::Tag_Player))
	{
		if (phoneNumberManager && !inEnableState)
		{
			inEnableState = true;
			//phoneNumberManager->
		}
	}
}

void APhoneNumberActor::OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
