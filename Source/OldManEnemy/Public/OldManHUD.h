// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FActiveInk.h"
#include "OldManHUD.generated.h"


UCLASS()
class OLDMANENEMY_API AOldManHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;
	void AddInk(FActiveInk NewInk);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	TArray<FActiveInk> ActiveInks; // 存储所有活跃墨渍
	
};
