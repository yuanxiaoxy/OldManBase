// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FEnemyLocationInfo.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AdEnemyManagerAgent.generated.h"

UCLASS()
class OLDMANENEMY_API AAdEnemyManagerAgent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAdEnemyManagerAgent();






	UFUNCTION(BlueprintCallable, Category = "EnemyManager/AdEnemy")
	void DrawAdEnemyDebug(TArray<FEnemyLocationInfo> infos, float lastTime);

};
