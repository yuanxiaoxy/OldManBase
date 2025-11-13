// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingletonBase/SingletonBase.h"
#include "OldManEnemyManager.generated.h"

class  AAdEnemyAIController;

UCLASS(Blueprintable, BlueprintType)
class OLDMANENEMY_API UOldManEnemyManager : public USingletonBase
{
	GENERATED_BODY()
	
public:	
	DECLARE_SINGLETON(UOldManEnemyManager)

	virtual void InitializeSingleton() override;
	
	UFUNCTION(BlueprintCallable, Category = "EnemyManager")
	void NotifyMonstersTracking();


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<AAdEnemyAIController*> Enemys;




private:
	UOldManEnemyManager();







};
