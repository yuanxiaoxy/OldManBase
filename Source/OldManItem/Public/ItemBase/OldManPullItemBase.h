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
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Drag")
	bool bIsBeingDragged;

public:
	UFUNCTION(BlueprintCallable)
	virtual void HandleMouseData(const FVector& ViewDirection, float Intensity);

	UFUNCTION(BlueprintCallable)
	virtual void StartDragging();

	UFUNCTION(BlueprintCallable)
	virtual void StopDragging();
};
