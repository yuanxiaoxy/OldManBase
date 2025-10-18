// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManItemBase.h"
#include "OldManInterectItemBase.generated.h"

USTRUCT(BlueprintType)
struct FOldManItemInteractData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    AActor* InteractingActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    FVector InteractionPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    FVector InteractionDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    float InteractionValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TArray<FString> CustomData;

    FOldManItemInteractData() {}

    FOldManItemInteractData(AActor* InActor, const FVector& InPoint = FVector::ZeroVector)
        : InteractingActor(InActor), InteractionPoint(InPoint)
    {
    }

    FOldManItemInteractData(AActor* InActor, float InValue)
        : InteractingActor(InActor), InteractionValue(InValue)
    {
    }
};

UCLASS()
class OLDMANITEM_API AOldManInterectItemBase : public AOldManItemBase
{
	GENERATED_BODY()

public:
    virtual void Interect(FOldManItemInteractData interectData);
	
};
