// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManItemBase.h"
#include "OldManCanBeAttackItemBase.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANITEM_API AOldManCanBeAttackItemBase : public AOldManItemBase
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AOldManCanBeAttackItemBase();

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "OnBeHitAttack")
	void BeAttacked();

	// 碰撞开始事件
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
};
