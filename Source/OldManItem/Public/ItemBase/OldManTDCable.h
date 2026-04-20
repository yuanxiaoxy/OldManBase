#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManCableBase.h"
#include "OldManTDCable.generated.h"

UCLASS()
class OLDMANITEM_API AOldManTDCable : public AOldManCableBase
{
    GENERATED_BODY()

public:
    AOldManTDCable();

    virtual bool IsBidirectional() const override { return true; }

    // 重写导航函数，移除 bReverseMovementDirection 的影响，确保双向移动自然
    virtual FVector GetStartLocation() const override;
    virtual FVector GetEndLocation() const override;
    virtual FVector GetDirectionAtPosition(const FVector& WorldPosition) const override;
    virtual FVector GetTangentAtPosition(const FVector& WorldPosition) const override;
    virtual FVector MoveAlongCable(const FVector& CurrentPosition, float Distance) const override;
    virtual FTransform GetTransformAtPosition(const FVector& WorldPosition) const override;

    virtual float FindNearestDistanceAlongSpline(const FVector& WorldPosition) const override;
    virtual FVector GetPositionAtDistance(float Distance) const override;
    virtual FVector GetTangentAtDistance(float Distance) const override;

protected:
    virtual FVector GetUpVectorAtDistance(float Distance) const override;
    virtual FVector GetRightVectorAtDistance(float Distance) const override;
};