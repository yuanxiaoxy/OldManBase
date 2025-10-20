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

UCLASS(Blueprintable)
class OLDMANITEM_API AOldManInterectItemBase : public AOldManItemBase
{
	GENERATED_BODY()

public:
    AOldManInterectItemBase();

public:
    virtual void Interect(FOldManItemInteractData interectData);
	
    // 用于互动的碰撞组件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* InteractionBox;

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "OnEnterTrigger")
    void OnEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintImplementableEvent, Category = "OnExitTrigger")
    void OnExitTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
    UFUNCTION()
        virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
        virtual void OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


};
