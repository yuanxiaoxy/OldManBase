// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalEventName.generated.h"

UCLASS(Blueprintable)
class OLDMANCONFIG_API UGlobalEventName : public UObject
{
	GENERATED_BODY()

public:
	static const FName Key_Player_OnDeath;
};
