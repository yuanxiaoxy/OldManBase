// Fill out your copyright notice in the Description page of Project Settings.


#include "PhoneNumberActor/PhoneNumberFinishActor.h"

void APhoneNumberFinishActor::BeTriggered(bool Immediately)
{
	canBeTrigger = true;

	if (Immediately)
	{
		Trigge();
	}
}

void APhoneNumberFinishActor::Trigge()
{
	hasBeenTriggered = true;
	OnTrigger();

	UE_LOG(LogTemp, Display, TEXT("Has BeenTrigger"));
}

void APhoneNumberFinishActor::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlayBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (!hasBeenTriggered && canBeTrigger)
	{
		Trigge();
	}
}

void APhoneNumberFinishActor::OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
