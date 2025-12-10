// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "SplinePipeArea.generated.h"

UCLASS(Blueprintable, BlueprintType)
class YOURMODULE_API ASplinePipeArea : public AActor
{
    GENERATED_BODY()

public:
    ASplinePipeArea();

    /* 半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
    float Radius = 50.f;

    /* 每厘米多少段碰撞 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe", meta = (ClampMin = 1))
    int SegmentsPerMeter = 20;

    /* 是否实时更新 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
    bool bLiveUpdate = true;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class USplineComponent* Spline;

    TArray<class UCapsuleComponent*> CollisionCapsules;

    void RebuildCollision();
#if WITH_EDITOR
    virtual void PostEditMove(bool bFinished) override;
#endif
};
