// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventManager/MyEventManager.h"
#include "GlobalTagName.h"
#include "GlobalEventName.h"
#include "SpecialAreaBase.generated.h"

UCLASS()
class OLDMANITEM_API ASpecialAreaBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpecialAreaBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// 用于互动的碰撞组件
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UBoxComponent* AreaBox;

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "OnEnterTrigger")
	void OnEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "OnExitTrigger")
	void OnExitTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UFUNCTION()
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
