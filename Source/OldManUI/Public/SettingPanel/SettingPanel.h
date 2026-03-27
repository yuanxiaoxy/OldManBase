// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OldManUIBase.h"
#include "SettingPanel.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class OLDMANUI_API USettingPanel : public UOldManUIBase
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(BlueprintReadWrite, Category = "MyCategory")
	float BGMVolume = 1;
	UPROPERTY(BlueprintReadWrite, Category = "MyCategory")
	float EffectVolume = 1;

	UFUNCTION(BlueprintCallable, Category = "MyCategory")
	void UpdateBGMToUI(float BGMVol);
	UFUNCTION(BlueprintCallable, Category = "MyCategory")
	void UpdateEffectToUI(float EffectVol);
	UFUNCTION(BlueprintCallable, Category = "MyCategory")
	void UpdateBGMToGame();
	UFUNCTION(BlueprintCallable, Category = "MyCategory")
	void UpdateEffectToGame();
};
