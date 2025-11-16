#include "ItemBase/OldManCableBase.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

AOldManCableBase::AOldManCableBase()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    CableSplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("CableSpline"));
    CableSplineComponent->SetupAttachment(RootComponent);

    // 设置默认样条点（起点和终点）
    CableSplineComponent->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
    CableSplineComponent->AddSplinePoint(FVector(500, 0, 0), ESplineCoordinateSpace::Local);

    // 设置样条线类型为线性，避免不必要的弯曲
    CableSplineComponent->SetSplinePointType(0, ESplinePointType::Linear);
    CableSplineComponent->SetSplinePointType(1, ESplinePointType::Linear);

    // 初始化默认值
    CableScale = FVector2D(0.1f, 0.1f);
    SegmentLength = 100.0f;
    bEnableCollision = true;
    bShowInEditor = true;
    TangentScale = 1.0f;
}

void AOldManCableBase::BeginPlay()
{
    Super::BeginPlay();

    // 在游戏开始时生成网格
    GenerateCableMesh();
}

void AOldManCableBase::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    // 在编辑器构建时更新可视化
    if (bShowInEditor)
    {
        UpdateCableVisualization();
    }
}

void AOldManCableBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 调试绘制
    if (bShowInEditor)
    {
        DrawDebugCable();
    }
}

void AOldManCableBase::DrawDebugCable()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // 绘制样条线
    int32 NumPoints = CableSplineComponent->GetNumberOfSplinePoints();
    for (int32 i = 0; i < NumPoints - 1; i++)
    {
        FVector StartPos = CableSplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        FVector EndPos = CableSplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::World);

        DrawDebugLine(World, StartPos, EndPos, FColor::Green, false, -1.0f, 0, 2.0f);
    }

    // 绘制样条点
    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector PointPos = CableSplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        DrawDebugSphere(World, PointPos, 10.0f, 8, FColor::Red, false, -1.0f, 0, 2.0f);
    }
}

FVector AOldManCableBase::GetStartLocation() const
{
    return CableSplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
}

FVector AOldManCableBase::GetEndLocation() const
{
    int32 LastPointIndex = CableSplineComponent->GetNumberOfSplinePoints() - 1;
    return CableSplineComponent->GetLocationAtSplinePoint(LastPointIndex, ESplineCoordinateSpace::World);
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

    // 检查是否在滑索范围内
    float DistanceToStart = FVector::Dist(NearestPosition, GetStartLocation());
    float DistanceToEnd = FVector::Dist(NearestPosition, GetEndLocation());
    float CableLength = GetCableLength();

    // 如果位置偏离滑索太远，限制在端点
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
    return CableSplineComponent->GetDirectionAtDistanceAlongSpline(NearestDistance, ESplineCoordinateSpace::World);
}

FVector AOldManCableBase::MoveAlongCable(const FVector& CurrentPosition, float Distance) const
{
    float CurrentDistance = FindNearestDistanceAlongSpline(CurrentPosition);
    float NewDistance = FMath::Clamp(CurrentDistance + Distance, 0.0f, GetCableLength());
    return GetPositionAtDistance(NewDistance);
}

float AOldManCableBase::FindNearestDistanceAlongSpline(const FVector& WorldPosition) const
{
    float InputKey = CableSplineComponent->FindInputKeyClosestToWorldLocation(WorldPosition);
    return CableSplineComponent->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

FVector AOldManCableBase::GetPositionAtDistance(float Distance) const
{
    return CableSplineComponent->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector AOldManCableBase::GetTangentAtDistance(float Distance) const
{
    return CableSplineComponent->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World) * TangentScale;
}

FVector AOldManCableBase::GetUpVectorAtDistance(float Distance) const
{
    return CableSplineComponent->GetUpVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
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

void AOldManCableBase::GenerateCableMesh()
{
    // 清除现有的网格组件
    ClearCableMesh();

    if (!CableMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("No cable mesh specified for %s"), *GetName());
        return;
    }

    float SplineLength = GetCableLength();

    // 如果样条线长度为0，则不生成网格
    if (SplineLength <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cable spline length is 0 for %s"), *GetName());
        return;
    }

    int32 NumSegments = FMath::Max(1, FMath::CeilToInt(SplineLength / SegmentLength));

    for (int32 i = 0; i < NumSegments; i++)
    {
        // 计算段的起始和结束距离
        float StartDistance = i * SegmentLength;
        float EndDistance = FMath::Min((i + 1) * SegmentLength, SplineLength);

        // 获取位置和切线
        FVector StartPos = GetPositionAtDistance(StartDistance) - GetActorLocation();
        FVector StartTangent = GetTangentAtDistance(StartDistance);
        FVector EndPos = GetPositionAtDistance(EndDistance) - GetActorLocation();
        FVector EndTangent = GetTangentAtDistance(EndDistance);

        // 创建样条网格体组件
        FName ComponentName = *FString::Printf(TEXT("SplineMeshComponent_%d"), i);
        USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(this, ComponentName);

        if (SplineMeshComponent)
        {
            SplineMeshComponent->SetMobility(EComponentMobility::Movable);
            SplineMeshComponent->SetupAttachment(RootComponent);
            SplineMeshComponent->RegisterComponent();

            // 设置静态网格体和材质
            SplineMeshComponent->SetStaticMesh(CableMesh);
            if (CableMaterial)
            {
                SplineMeshComponent->SetMaterial(0, CableMaterial);
            }

            // 设置样条点 - 使用正确的坐标空间
            SplineMeshComponent->SetStartPosition(StartPos);
            SplineMeshComponent->SetStartTangent(StartTangent);
            SplineMeshComponent->SetEndPosition(EndPos);
            SplineMeshComponent->SetEndTangent(EndTangent);

            // 设置前向轴
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

            // 对于负轴，可能需要旋转网格
            if (CableForwardAxis == ECableForwardAxis::NegativeX ||
                CableForwardAxis == ECableForwardAxis::NegativeY ||
                CableForwardAxis == ECableForwardAxis::NegativeZ)
            {
                FRotator Rotation(0, 180, 0);
                SplineMeshComponent->SetRelativeRotation(Rotation);
            }

            // 设置缩放
            SplineMeshComponent->SetStartScale(FVector2D(CableScale));
            SplineMeshComponent->SetEndScale(FVector2D(CableScale));

            // 设置碰撞
            if (bEnableCollision)
            {
                SplineMeshComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
                SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
            else
            {
                SplineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }

            SplineMeshComponents.Add(SplineMeshComponent);

            // 调试信息
            UE_LOG(LogTemp, Verbose, TEXT("Created cable segment %d: Start=%s, End=%s"),
                i, *StartPos.ToString(), *EndPos.ToString());
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Generated %d cable segments for %s"), NumSegments, *GetName());
}

void AOldManCableBase::ClearCableMesh()
{
    for (USplineMeshComponent* SplineMeshComponent : SplineMeshComponents)
    {
        if (SplineMeshComponent)
        {
            SplineMeshComponent->DestroyComponent();
        }
    }
    SplineMeshComponents.Empty();
}

void AOldManCableBase::UpdateCableVisualization()
{
    // 在编辑器中生成或更新网格
    if (bShowInEditor)
    {
        GenerateCableMesh();
    }
    else
    {
        ClearCableMesh();
    }
}

#if WITH_EDITOR
void AOldManCableBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    // 如果修改了这些属性，更新可视化
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableMesh) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableMaterial) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableForwardAxis) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, CableScale) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, SegmentLength) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bEnableCollision) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, bShowInEditor) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(AOldManCableBase, TangentScale) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(USplineComponent, SplineCurves)) // 样条曲线改变时
    {
        UpdateCableVisualization();
    }
}
#endif