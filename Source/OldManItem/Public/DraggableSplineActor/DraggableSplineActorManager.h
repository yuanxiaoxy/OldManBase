// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "DraggableSplineActor/DraggableSplineActor.h"
#include "DraggableSplineActorManager.generated.h"

// 通用事件数据结构
USTRUCT(BlueprintType)
struct FDraggableItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggableActorManager")
	TArray<ADraggableSplineActor*> DraggableSplineActors;
};

class ADraggableSplineActor;

UCLASS(Blueprintable)
class OLDMANITEM_API ADraggableSplineActorManager : public AActor
{
	GENERATED_BODY()

public:
	ADraggableSplineActorManager();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggableActorManager")
	TMap<FString, FDraggableItemData> DraggableActorMap;

public:
	UFUNCTION(BlueprintCallable)
	void ResetDraggableSplineActorPos(FString GroupName);
	
};
