#include "ItemBase/OldManTDCable.h"
#include "Components/SplineComponent.h"

AOldManTDCable::AOldManTDCable()
{
    // 双向滑索不需要反转方向标志，始终沿样条自然方向处理移动
    bReverseMovementDirection = false;
}

FVector AOldManTDCable::GetStartLocation() const
{
    return CableSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
}

FVector AOldManTDCable::GetEndLocation() const
{
    int32 LastPointIndex = CableSplineComponent->GetNumberOfSplinePoints() - 1;
    return CableSplineComponent->GetLocationAtSplinePoint(LastPointIndex, ESplineCoordinateSpace::World);
}

FVector AOldManTDCable::GetDirectionAtPosition(const FVector& WorldPosition) const
{
    float NearestDistance = FindNearestDistanceAlongSpline(WorldPosition);
    return CableSplineComponent->GetDirectionAtDistanceAlongSpline(NearestDistance, ESplineCoordinateSpace::World);
}

FVector AOldManTDCable::GetTangentAtPosition(const FVector& WorldPosition) const
{
    float NearestDistance = FindNearestDistanceAlongSpline(WorldPosition);
    return CableSplineComponent->GetTangentAtDistanceAlongSpline(NearestDistance, ESplineCoordinateSpace::World) * TangentScale;
}

FVector AOldManTDCable::MoveAlongCable(const FVector& CurrentPosition, float Distance) const
{
    float CurrentDistance = FindNearestDistanceAlongSpline(CurrentPosition);
    float NewDistance = FMath::Clamp(CurrentDistance + Distance, 0.0f, GetCableLength());
    return GetPositionAtDistance(NewDistance);
}

FTransform AOldManTDCable::GetTransformAtPosition(const FVector& WorldPosition) const
{
    float Distance = FindNearestDistanceAlongSpline(WorldPosition);
    FVector Location = CableSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector Tangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    FVector UpVector = CableSplineComponent->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    Tangent.Normalize();
    UpVector.Normalize();
    FVector RightVector = FVector::CrossProduct(UpVector, Tangent).GetSafeNormal();
    FVector RealUpVector = FVector::CrossProduct(Tangent, RightVector).GetSafeNormal();
    FMatrix RotationMatrix(Tangent, RightVector, RealUpVector, FVector::ZeroVector);
    FRotator Rotation = RotationMatrix.Rotator();
    // 双向滑索不在类内部翻转旋转，翻转由状态层根据移动方向处理
    return FTransform(Rotation, Location);
}

float AOldManTDCable::FindNearestDistanceAlongSpline(const FVector& WorldPosition) const
{
    float InputKey = CableSplineComponent->FindInputKeyClosestToWorldLocation(WorldPosition);
    return CableSplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

FVector AOldManTDCable::GetPositionAtDistance(float Distance) const
{
    return CableSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector AOldManTDCable::GetTangentAtDistance(float Distance) const
{
    return CableSplineComponent->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) * TangentScale;
}

FVector AOldManTDCable::GetUpVectorAtDistance(float Distance) const
{
    return CableSplineComponent->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector AOldManTDCable::GetRightVectorAtDistance(float Distance) const
{
    return CableSplineComponent->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}