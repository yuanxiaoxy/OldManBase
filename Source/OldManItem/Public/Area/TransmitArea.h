// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Area/SpecialAreaBase.h"
#include "TransmitArea.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANITEM_API ATransmitArea : public ASpecialAreaBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransmitPosition")
	USceneComponent* TransmitPosition;

	ATransmitArea();

protected:
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransmitPosition")
	float DelayTime = -1;

};
