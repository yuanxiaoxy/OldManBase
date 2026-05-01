// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/ActorSingletonBase.h"
#include "PostProcessManager.generated.h"

//渐隐结束委托
DECLARE_DYNAMIC_DELEGATE(FPostCallbackDelegate);

UCLASS()
class OLDMANITEM_API APostProcessManager : public AActorSingletonBase
{
	GENERATED_BODY()
	
	DECLARE_ACTOR_SINGLETON(APostProcessManager)

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AllSpeed = 10;

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

	UFUNCTION(BlueprintCallable)
	void SetLightStrength(float Value);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetLightStrength(float Value);

	UFUNCTION(BlueprintCallable)
	void Fade2None();
	UFUNCTION(BlueprintImplementableEvent)
	void BP_Fade2None();

	UFUNCTION(BlueprintCallable)
	void FlashRed(float Time);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_FlashRed(float Time);

	UFUNCTION(BlueprintCallable)
	void FlashGreen(float Time);
	UFUNCTION(BlueprintImplementableEvent)
	void BP_FlashGreen(float Time);
};
