#pragma once
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalTagName.generated.h"

UCLASS(Blueprintable)
class OLDMANCONFIG_API UGlobalTagName : public UObject
{
	GENERATED_BODY()

public:
	static const FName Tag_Player;
	static const FName Tag_InterectItem;
};
