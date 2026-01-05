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

    const int32 NumNodes = Spline->GetNumberOfSplinePoints();
    if (NumNodes < 2) return;

    /* 保证数组长度一致，否则用默认半径 */
    if (NodeRadii.Num() != NumNodes)
    {
        NodeRadii.SetNum(NumNodes);
        for (float& R : NodeRadii) R = Radius;   // 默认半径
    }

    const float SplineLen = Spline->GetSplineLength();
    const int32   NumSeg = FMath::Max(1, FMath::RoundToInt(SplineLen * SegmentsPerMeter * 0.01f));
    const float   Step = SplineLen / NumSeg;

    auto GetRadiusAtDistance = [&](float D)->float
        {
            /* 先把距离映射到 0-1 的输入键 */
            const float Key = Spline->GetInputKeyValueAtDistanceAlongSpline(D);
            /* 再映射到节点索引 */
            const int32 Idx = FMath::Clamp(FMath::FloorToInt(Key), 0, NumNodes - 2);
            const float Alpha = (Key - Idx);   // 0-1 插值系数
            return FMath::Lerp(NodeRadii[Idx], NodeRadii[Idx + 1], Alpha);
        };

    for (int32 i = 0; i < NumSeg; ++i)
    {
        const float t1 = i * Step;
        const float t2 = (i + 1) * Step;
        FVector pos1 = Spline->GetLocationAtDistanceAlongSpline(t1, ESplineCoordinateSpace::Local);
        FVector pos2 = Spline->GetLocationAtDistanceAlongSpline(t2, ESplineCoordinateSpace::Local);
        FVector dir = (pos2 - pos1).GetSafeNormal();
        float segLen = FVector::Dist(pos1, pos2);

        /* 两端半径 */
        float r1 = GetRadiusAtDistance(t1);
        float r2 = GetRadiusAtDistance(t2);
        float avgR = (r1 + r2) * 0.5f;

        auto* Caps = NewObject<UCapsuleComponent>(this);
        Caps->RegisterComponent();
        Caps->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
        Caps->SetCapsuleRadius(avgR);
        Caps->SetCapsuleHalfHeight(segLen * 0.5f);
        Caps->SetRelativeLocation(pos1 + dir * segLen * 0.5f);
        Caps->SetRelativeRotation(dir.ToOrientationRotator());

        Caps->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Caps->SetCollisionResponseToAllChannels(ECR_Ignore);
        Caps->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

        CollisionCapsules.Add(Caps);
    }
}
//void ASplineDeathArea::RebuildCollision()
//{
//    for (auto* Caps : CollisionCapsules) Caps->DestroyComponent();
//    CollisionCapsules.Reset();
//
//    if (!Spline || Spline->GetNumberOfSplinePoints() < 2) return;
//
//    const float SplineLen = Spline->GetSplineLength();
//    const int32 NumSeg = FMath::Max(1, FMath::RoundToInt(SplineLen * SegmentsPerMeter * 0.01f));
//    const float Step = SplineLen / NumSeg;
//
//    for (int32 i = 0; i < NumSeg; ++i)
//    {
//        const float t1 = i * Step;
//        const float t2 = (i + 1) * Step;
//        FVector pos1 = Spline->GetLocationAtDistanceAlongSpline(t1, ESplineCoordinateSpace::Local);
//        FVector pos2 = Spline->GetLocationAtDistanceAlongSpline(t2, ESplineCoordinateSpace::Local);
//        FVector dir = (pos2 - pos1).GetSafeNormal();
//        float segLen = FVector::Dist(pos1, pos2);
//
//        auto* Caps = NewObject<UCapsuleComponent>(this);
//        Caps->RegisterComponent();
//        Caps->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
//        Caps->SetCapsuleRadius(Radius);
//        Caps->SetCapsuleHalfHeight(segLen * 0.5f);
//        Caps->SetRelativeLocation(pos1 + dir * segLen * 0.5f);
//        Caps->SetRelativeRotation(dir.ToOrientationRotator());
//        Caps->SetCollisionEnabled(ECollisionEnabled::QueryOnly);          // 无物理
//        Caps->SetCollisionResponseToAllChannels(ECR_Ignore);              // 先全部忽略
//        Caps->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
//        CollisionCapsules.Add(Caps);
//    }
//}

#if WITH_EDITOR
void ASplineDeathArea::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);
    if (bLiveUpdate) RebuildCollision();
}
#endif