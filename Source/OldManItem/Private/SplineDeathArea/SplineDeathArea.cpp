// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineDeathArea/SplineDeathArea.h"
#include "Components/CapsuleComponent.h"

ASplineDeathArea::ASplineDeathArea()
{
    PrimaryActorTick.bCanEverTick = false;
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = Spline;
}

void ASplineDeathArea::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RebuildCollision();
}

void ASplineDeathArea::BeginPlay()
{
    Super::BeginPlay();
    RebuildCollision();
}

void ASplineDeathArea::RebuildCollision()
{
    for (auto* Caps : CollisionCapsules) Caps->DestroyComponent();
    CollisionCapsules.Reset();

    if (!Spline || Spline->GetNumberOfSplinePoints() < 2) return;

    const float SplineLen = Spline->GetSplineLength();
    const int32 NumSeg = FMath::Max(1, FMath::RoundToInt(SplineLen * SegmentsPerMeter * 0.01f));
    const float Step = SplineLen / NumSeg;

    for (int32 i = 0; i < NumSeg; ++i)
    {
        const float t1 = i * Step;
        const float t2 = (i + 1) * Step;
        FVector pos1 = Spline->GetLocationAtDistanceAlongSpline(t1, ESplineCoordinateSpace::Local);
        FVector pos2 = Spline->GetLocationAtDistanceAlongSpline(t2, ESplineCoordinateSpace::Local);
        FVector dir = (pos2 - pos1).GetSafeNormal();
        float segLen = FVector::Dist(pos1, pos2);

        auto* Caps = NewObject<UCapsuleComponent>(this);
        Caps->RegisterComponent();
        Caps->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
        Caps->SetCapsuleRadius(Radius);
        Caps->SetCapsuleHalfHeight(segLen * 0.5f);
        Caps->SetRelativeLocation(pos1 + dir * segLen * 0.5f);
        Caps->SetRelativeRotation(dir.ToOrientationRotator());
        Caps->SetCollisionEnabled(ECollisionEnabled::QueryOnly);          // 无物理
        Caps->SetCollisionResponseToAllChannels(ECR_Ignore);              // 先全部忽略
        Caps->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        CollisionCapsules.Add(Caps);
    }
}

#if WITH_EDITOR
void ASplineDeathArea::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);
    if (bLiveUpdate) RebuildCollision();
}
#endif