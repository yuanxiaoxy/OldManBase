// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManInterectItemBase.h"
#include "PhoneNumberActor.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANITEM_API APhoneNumberActor : public AOldManInterectItemBase
{
	GENERATED_BODY()

public:
	APhoneNumberActor();


public:
	UFUNCTION()
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


};
