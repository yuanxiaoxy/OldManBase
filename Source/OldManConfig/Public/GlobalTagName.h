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
	UFUNCTION(BlueprintCallable, Category = "GlobalTagName")
	static FName GetTag_Player() { return Tag_Player; }

	static const FName Tag_InterectItem;
	UFUNCTION(BlueprintCallable, Category = "GlobalTagName")
	static FName GetTag_InterectItem() { return Tag_InterectItem; }
	static const FName Tag_BeDetcedItem;
	UFUNCTION(BlueprintCallable, Category = "GlobalTagName")
	static FName GetTag_BeDetcedItem() { return Tag_BeDetcedItem; }
	static const FName Tag_DetcedItem;
	UFUNCTION(BlueprintCallable, Category = "GlobalTagName")
	static FName GetTag_DetcedItem() { return Tag_DetcedItem; }

	static const FName Tag_SlopeGround;
	UFUNCTION(BlueprintCallable, Category = "GlobalTagName")
	static FName GetTag_SlopeGround() { return Tag_SlopeGround; }
};
