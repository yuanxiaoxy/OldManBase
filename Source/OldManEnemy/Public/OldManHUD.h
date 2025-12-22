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
	void AddInk(UTexture2D* InkTexture, FVector2D ScreenPosition, float DisplayTime);
private:
	TArray<FActiveInk> ActiveInks; // 存储所有活跃墨渍
	
};
