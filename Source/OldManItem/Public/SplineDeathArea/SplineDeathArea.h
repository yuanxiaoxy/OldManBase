// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "SplineDeathArea.generated.h"

UCLASS(Blueprintable, BlueprintType)
class OLDMANITEM_API ASplineDeathArea : public AActor
{
    GENERATED_BODY()

public:
    ASplineDeathArea();

    /* 半径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
    float Radius = 50.f;

    /* 每厘米多少段碰撞 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe", meta = (ClampMin = 0.01f))
    float SegmentsPerMeter = 0.1f;   // 0.1 = 每 10 m 一段

    /* 是否实时更新 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
    bool bLiveUpdate = true;

    /* 节点半径数组 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pipe")
    TArray<float> NodeRadii;   // 长度必须与 Spline 点数一致

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