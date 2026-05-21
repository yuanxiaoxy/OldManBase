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
	UPROPERTY(BlueprintReadOnly, Category = "Drag")
	bool bCouldPull = true;

	UPROPERTY(BlueprintReadOnly, Category = "Drag")
	bool bIsBeingDragged;

	UFUNCTION(BlueprintCallable)
	virtual void HandleMouseData(const FVector& ViewDirection, float Intensity);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_HandleMouseData(const FVector& ViewDirection, float Intensity);

	UFUNCTION(BlueprintCallable)
	virtual void StartDragging();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStartDragging();

	UFUNCTION(BlueprintCallable)
	virtual void StopDragging();

	UFUNCTION(BlueprintImplementableEvent)
	void OnStopDragging();

	UFUNCTION(BlueprintCallable)
	void C_OnBeChecked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnBeChecked();

	UFUNCTION(BlueprintCallable)
	void C_OnDismissChecked();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDismissChecked();
};
