// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManItemBase.h"
#include "OldManPullItemBase.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANITEM_API AOldManPullItemBase : public AOldManItemBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	virtual void HandleMouseData(const FVector& ViewDirection, float Intensity);
};
