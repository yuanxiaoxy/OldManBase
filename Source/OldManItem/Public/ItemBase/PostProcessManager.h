// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/ActorSingletonBase.h"
#include "PostProcessManager.generated.h"


UCLASS()
class OLDMANITEM_API APostProcessManager : public AActorSingletonBase
{
	GENERATED_BODY()
	
	DECLARE_ACTOR_SINGLETON(APostProcessManager)

public:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "PostProcessManager", meta = (DisplayName = "Get PostProcessManager"))
	static APostProcessManager* GetPostProcessManager() { return GetInstance(); }

	UFUNCTION(BlueprintCallable)
	void StartPostProcessAnim();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_StartPostProcessAnim();

	UFUNCTION(BlueprintCallable)
	void StopPostProcessAnim();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_StopPostProcessAnim();
};
