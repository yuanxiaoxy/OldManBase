#include "ItemBase/OldManCableBase.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

AOldManCableBase::AOldManCableBase()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    CableSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("CableSpline"));
    CableSplineComponent->SetupAttachment(RootComponent);

    // 初始化两个点
    CableSplineComponent->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
    CableSplineComponent->AddSplinePoint(FVector(500, 0, 0), ESplineCoordinateSpace::Local);
    CableSplineComponent->SetSplinePointType(0, ESplinePointType::Linear);
    CableSplineComponent->SetSplinePointType(1, ESplinePointType::Linear);

    CableScale = FVector2D(0.1f, 0.1f);
    SegmentLength = 100.0f;
    bEnableCollision = true;
    bShowInEditor = true;
    TangentScale = 1.0f;
    bReverseMovementDirection = false;
    CollisionProfileName = TEXT("BlockAll");
    bGenerateOverlapEvents = false;
}

void AOldManCableBase::BeginPlay()
{
    Super::BeginPlay();
    GenerateCableMesh();

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            UpdateCableCollision();
        }, 0.01f, false);
}

void AOldManCableBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    if (bShowInEditor)
    {
        UpdateCableVisualization();
    }
}

// ============ Cable Navigation API ============

FVector AOldManCableBase::GetStartLocation() const
{
    if (bReverseMovementDirection)
    {
        int32 LastPointIndex = CableSplineComponent->GetNumberOfSplinePoints() - 1;
        return CableSplineComponent->GetLocationAtSplinePoint(LastPointIndex, ESplineCoordinateSpace::World);
    }
    else
    {
        return CableSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    }
}

FVector AOldManCableBase::GetEndLocation() const
{
    if (bReverseMovementDirection)
    {
        return CableSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
    }
    else
    {
        int32 LastPointIndex = CableSplineComponent->GetNumberOfSplinePoints() - 1;
        return CableSplineComponent->GetLocationAtSplinePoint(LastPointIndex, ESplineCoordinateSpace::World);
    }
}

float AOldManCableBase::GetCableLength() const
{
    return CableSplineComponent->GetSplineLength();
}

FVector AOldManCableBase::FindNearestPosition(const FVector& WorldPosition) const
{
    float NearestDistance = FindNearestDistanceAlongSpline(WorldPosition);
    return GetPositionAtDistance(NearestDistance);
}

FVector AOldManCableBase::ProjectPositionToCable(const FVector& WorldPosition) const
{
    return FindNearestPosition(WorldPosition);
}

FVector AOldManCableBase::ClampPositionToCable(const FVector& WorldPosition) const
{
    FVector NearestPosition = FindNearestPosition(WorldPosition);
    float DistanceToStart = FVector::Dist(NearestPosition, GetStartLocation());
    float DistanceToEnd = FVector::Dist(NearestPosition, GetEndLocation());
    float CableLength = GetCableLength();

    if (DistanceToStart > CableLength * 1.1f || DistanceToEnd > CableLength * 1.1f)
    {
        return (DistanceToStart < DistanceToEnd) ? GetStartLocation() : GetEndLocation();
    }
    return NearestPosition;
}

bool AOldManCableBase::IsAtEndOfCable(const FVector& WorldPosition) const
{
    float DistanceToStart = FVector::Dist(WorldPosition, GetStartLocation());
    float DistanceToEnd = FVector::Dist(WorldPosition, GetEndLocation());
    return (DistanceToStart < EndDetectionDistance) || (DistanceToEnd < EndDetectionDistance);
}

FVector AOldManCableBase::GetDirectionAtPosition(const FVector& WorldPosition) const
{
    float NearestDistance = FindNearestDistanceAlongSpline(WorldPosition);
    FVector Direction;
    if (bReverseMovementDirection)
    {
        float AdjustedDistance = GetCableLength() - NearestDistance;
        Direction = CableSplineComponent->GetDirectionAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
        Direction = -Direction;
    }
    else
    {
        Direction = CableSplineComponent->GetDirectionAtDistanceAlongSpline(NearestDistance, ESplineCoordinateSpace::World);
    }
    return Direction;
}

FVector AOldManCableBase::GetTangentAtPosition(const FVector& WorldPosition) const
{
    float NearestDistance = FindNearestDistanceAlongSpline(WorldPosition);
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - NearestDistance) : NearestDistance;
    FVector Tangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World) * TangentScale;
    if (bReverseMovementDirection) Tangent = -Tangent;
    return Tangent;
}

FVector AOldManCableBase::MoveAlongCable(const FVector& CurrentPosition, float Distance) const
{
    float CurrentDistance = FindNearestDistanceAlongSpline(CurrentPosition);
    float DirectionSign = 1.0f;
    if (!IsBidirectional())
    {
        // 单向滑索：固定移动方向，由 bReverseMovementDirection 决定符号
        DirectionSign = bReverseMovementDirection ? -1.0f : 1.0f;
        // 强制使用绝对值的距离，确保方向固定
        Distance = FMath::Abs(Distance) * DirectionSign;
    }
    // 双向滑索：Distance 符号表示玩家输入方向
    float NewDistance = FMath::Clamp(CurrentDistance + Distance, 0.0f, GetCableLength());
    return GetPositionAtDistance(NewDistance);
}

FVector AOldManCableBase::GetCharacterPositionOnCable(const FVector& WorldPosition, float CharacterRadius) const
{
    FVector NearestPosition = FindNearestPosition(WorldPosition);
    FVector UpVector = GetUpVectorAtPosition(WorldPosition);
    float TotalRadius = CalculateCableRadius() + CharacterRadius;
    return NearestPosition + UpVector * TotalRadius;
}

// ============ Cable Visualization ============

void AOldManCableBase::GenerateCableMesh()
{
    ClearCableMesh();
    if (!CableMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No cable mesh specified for %s"), *GetName());
        return;
    }

    float SplineLength = GetCableLength();
    if (SplineLength <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cable spline length is 0 for %s"), *GetName());
        return;
    }

    int32 NumSegments = FMath::Max(1, FMath::CeilToInt(SplineLength / SegmentLength));

    for (int32 i = 0; i < NumSegments; i++)
    {
        float StartDistance = i * SegmentLength;
        float EndDistance = FMath::Min((i + 1) * SegmentLength, SplineLength);

        FVector StartPos = CableSplineComponent->GetLocationAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::Local);
        FVector StartTangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(StartDistance, ESplineCoordinateSpace::Local) * TangentScale;
        FVector EndPos = CableSplineComponent->GetLocationAtDistanceAlongSpline(EndDistance, ESplineCoordinateSpace::Local);
        FVector EndTangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(EndDistance, ESplineCoordinateSpace::Local) * TangentScale;

        FName ComponentName = *FString::Printf(TEXT("SplineMeshComponent_%d"), i);
        USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, ComponentName);

        if (SplineMeshComponent)
        {
            SplineMeshComponent->SetMobility(EComponentMobility::Movable);
            SplineMeshComponent->SetupAttachment(RootComponent);
            SplineMeshComponent->RegisterComponent();

            SplineMeshComponent->SetStaticMesh(CableMesh);
            if (CableMaterial) SplineMeshComponent->SetMaterial(0, CableMaterial);

            SplineMeshComponent->SetStartPosition(StartPos);
            SplineMeshComponent->SetStartTangent(StartTangent);
            SplineMeshComponent->SetEndPosition(EndPos);
            SplineMeshComponent->SetEndTangent(EndTangent);

            ESplineMeshAxis::Type ForwardAxis = ESplineMeshAxis::X;
            switch (CableForwardAxis)
            {
            case ECableForwardAxis::X: ForwardAxis = ESplineMeshAxis::X; break;
            case ECableForwardAxis::Y: ForwardAxis = ESplineMeshAxis::Y; break;
            case ECableForwardAxis::Z: ForwardAxis = ESplineMeshAxis::Z; break;
            case ECableForwardAxis::NegativeX: ForwardAxis = ESplineMeshAxis::X; break;
            case ECableForwardAxis::NegativeY: ForwardAxis = ESplineMeshAxis::Y; break;
            case ECableForwardAxis::NegativeZ: ForwardAxis = ESplineMeshAxis::Z; break;
            }
            SplineMeshComponent->SetForwardAxis(ForwardAxis);

            if (CableForwardAxis == ECableForwardAxis::NegativeX ||
                CableForwardAxis == ECableForwardAxis::NegativeY ||
                CableForwardAxis == ECableForwardAxis::NegativeZ)
            {
                SplineMeshComponent->SetRelativeRotation(FRotator(0, 180, 0));
            }

            SplineMeshComponent->SetStartScale(CableScale);
            SplineMeshComponent->SetEndScale(CableScale);

            if (bEnableCollision)
            {
                SplineMeshComponent->SetCollisionProfileName(CollisionProfileName);
                SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                SplineMeshComponent->SetGenerateOverlapEvents(bGenerateOverlapEvents);
            }
            else
            {
                SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }

            SplineMeshComponents.Add(SplineMeshComponent);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("Generated %d cable segments for %s"), NumSegments, *GetName());
}

void AOldManCableBase::ClearCableMesh()
{
    for (USplineMeshComponent* Comp : SplineMeshComponents)
    {
        if (Comp) Comp->DestroyComponent();
    }
    SplineMeshComponents.Empty();
}

void AOldManCableBase::UpdateCableVisualization()
{
    if (bShowInEditor)
        GenerateCableMesh();
    else
        ClearCableMesh();
}

// ============ Cable Collision ============

void AOldManCableBase::UpdateCableCollision()
{
    for (USplineMeshComponent* Comp : SplineMeshComponents)
    {
        if (!Comp) continue;

        if (bEnableCollision)
        {
            Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            if (!CollisionProfileName.IsNone())
                Comp->SetCollisionProfileName(CollisionProfileName);
            else
            {
                Comp->SetCollisionResponseToAllChannels(ECR_Block);
                Comp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
                Comp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            }
            Comp->SetCollisionObjectType(ECC_WorldDynamic);
            Comp->SetGenerateOverlapEvents(bGenerateOverlapEvents);
            Comp->SetNotifyRigidBodyCollision(true);
            Comp->RecreatePhysicsState();
        }
        else
        {
            Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            Comp->RecreatePhysicsState();
        }
    }
}

void AOldManCableBase::SetCollisionEnabled(bool bEnable)
{
    bEnableCollision = bEnable;
    UpdateCableCollision();
}

// ============ Helper Functions ============

float AOldManCableBase::FindNearestDistanceAlongSpline(const FVector& WorldPosition) const
{
    float InputKey = CableSplineComponent->FindInputKeyClosestToWorldLocation(WorldPosition);
    float Distance = CableSplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
    if (bReverseMovementDirection) Distance = GetCableLength() - Distance;
    return Distance;
}

FVector AOldManCableBase::GetPositionAtDistance(float Distance) const
{
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - Distance) : Distance;
    return CableSplineComponent->GetLocationAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
}

FVector AOldManCableBase::GetTangentAtDistance(float Distance) const
{
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - Distance) : Distance;
    FVector Tangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World) * TangentScale;
    if (bReverseMovementDirection) Tangent = -Tangent;
    return Tangent;
}

FVector AOldManCableBase::GetUpVectorAtPosition(const FVector& WorldPosition) const
{
    float Distance = FindNearestDistanceAlongSpline(WorldPosition);
    return GetUpVectorAtDistance(Distance);
}

FVector AOldManCableBase::GetUpVectorAtDistance(float Distance) const
{
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - Distance) : Distance;
    return CableSplineComponent->GetUpVectorAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
}

FVector AOldManCableBase::GetRightVectorAtPosition(const FVector& WorldPosition) const
{
    float Distance = FindNearestDistanceAlongSpline(WorldPosition);
    return GetRightVectorAtDistance(Distance);
}

FVector AOldManCableBase::GetRightVectorAtDistance(float Distance) const
{
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - Distance) : Distance;
    return CableSplineComponent->GetRightVectorAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
}

FTransform AOldManCableBase::GetTransformAtPosition(const FVector& WorldPosition) const
{
    float Distance = FindNearestDistanceAlongSpline(WorldPosition);
    float AdjustedDistance = bReverseMovementDirection ? (GetCableLength() - Distance) : Distance;
    FVector Location = CableSplineComponent->GetLocationAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
    FVector Tangent = CableSplineComponent->GetTangentAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
    FVector UpVector = CableSplineComponent->GetUpVectorAtDistanceAlongSpline(AdjustedDistance, ESplineCoordinateSpace::World);
    Tangent.Normalize();
    UpVector.Normalize();
    FVector RightVector = FVector::CrossProduct(UpVector, Tangent).GetSafeNormal();
    FVector RealUpVector = FVector::CrossProduct(Tangent, RightVector).GetSafeNormal();
    FMatrix RotationMatrix(Tangent, RightVector, RealUpVector, FVector::ZeroVector);
    FRotator Rotation = RotationMatrix.Rotator();
    // 注意：单向滑索不在类内部翻转旋转，翻转由状态根据 IsBidirectional 处理
    return FTransform(Rotation, Location);
}

FRotator AOldManCableBase::GetRotationAtPosition(const FVector& WorldPosition) const
{
    return GetTransformAtPosition(WorldPosition).Rotator();
}

FVector AOldManCableBase::GetForwardAxisVector() const
{
    switch (CableForwardAxis)
    {
    case ECableForwardAxis::X: return FVector(1, 0, 0);
    case ECableForwardAxis::Y: return FVector(0, 1, 0);
    case ECableForwardAxis::Z: return FVector(0, 0, 1);
    case ECableForwardAxis::NegativeX: return FVector(-1, 0, 0);
    case ECableForwardAxis::NegativeY: return FVector(0, -1, 0);
    case ECableForwardAxis::NegativeZ: return FVector(0, 0, -1);
    default: return FVector(1, 0, 0);
    }
}

float AOldManCableBase::CalculateCableRadius() const
{
    return CableRadius > 0.0f ? CableRadius : 30.0f;
}

#if WITH_EDITOR
void AOldManCableBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableMesh) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableMaterial) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableForwardAxis) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableScale) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, SegmentLength) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bEnableCollision) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bShowInEditor) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, TangentScale) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bReverseMovementDirection) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CollisionProfileName) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bGenerateOverlapEvents) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves))
    {
        UpdateCableVisualization();
        if (PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bEnableCollision) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CollisionProfileName) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bGenerateOverlapEvents))
        {
            UpdateCableCollision();
        }
    }
}
#endif