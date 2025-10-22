// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManInterectItemBase.h"
#include "PhoneNumberActor.generated.h"

class APhoneNumberManager;

UCLASS(Blueprintable)
class OLDMANITEM_API APhoneNumberActor : public AOldManInterectItemBase
{
	GENERATED_BODY()

public:
	APhoneNumberActor();

public:
	UFUNCTION()
	void InitPhoneNumberActor(int number, FString groupName, APhoneNumberManager* PhoneNumberManager);

	UFUNCTION()
	void Reset();
	UFUNCTION(BlueprintImplementableEvent, Category = "OnReset")
	void OnReset();

protected:
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:
	bool inEnableState;

	int thisNumber;
	FString thisGroup;

	APhoneNumberManager* phoneNumberManager;
};
