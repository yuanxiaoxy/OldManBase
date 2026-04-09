// DraggableSplineActor.cpp
#include "DraggableSplineActor/DraggableSplineActor.h"
#include "Engine/World.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

ADraggableSplineActor::ADraggableSplineActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);

    CurrentSplinePosition = 0.0f;
    bIsBeingDragged = false;
    MovementAlpha = 1.0f;
    SmoothedMovementDirection = FVector::ZeroVector;
    inAutoBack = false;

    Tags.Add("DragableItem");
}

void ADraggableSplineActor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    InitialMeshLocation = SplineComponent->GetLocationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    InitialMeshRotation = SplineComponent->GetRotationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    SetStartPosition();

    TargetLocation = MeshComponent->GetComponentLocation();
    TargetRotation = MeshComponent->GetComponentRotation();
}

void ADraggableSplineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 自动回弹：以恒定速度移动 CurrentSplinePosition 回 DragStartPos
    if (inAutoBack && IfHasAutoBack)
    {
        // 如果还没到达起点，继续移动
        if (CurrentSplinePosition != DragStartPos)
        {
            float Direction = (DragStartPos > CurrentSplinePosition) ? 1.0f : -1.0f;
            float Step = AutoBackSpeed * DeltaTime;
            float NewPosition = CurrentSplinePosition + Direction * Step;

            // 检查是否越过目标
            if ((Direction > 0 && NewPosition >= DragStartPos) || (Direction < 0 && NewPosition <= DragStartPos))
            {
                NewPosition = DragStartPos;
            }

            CurrentSplinePosition = NewPosition;

            // 更新目标位置和旋转
            TargetLocation = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
            TargetRotation = SplineComponent->GetRotationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
            MovementAlpha = 0.0f;   // 重新开始平滑移动
        }

        // 如果已经到达起点，且平滑插值已完成，则结束自动回弹
        if (CurrentSplinePosition == DragStartPos && MovementAlpha >= 1.0f)
        {
            StopAutoBack();
        }
    }

    // 平滑移动插值
    if (MovementAlpha < 1.0f)
    {
        MovementAlpha = FMath::Min(MovementAlpha + DeltaTime * 8.0f, 1.0f);
        FVector NewLocation = FMath::Lerp(MeshComponent->GetComponentLocation(), TargetLocation, MovementAlpha);
        FRotator NewRotation = FMath::Lerp(MeshComponent->GetComponentRotation(), TargetRotation, MovementAlpha);
        SetMeshPositionAndRotation(NewLocation, NewRotation);
    }

    // 调试绘制
    if (bShowDebugVisualization && SplineComponent)
    {
        const int32 NumSegments = 50;
        for (int32 i = 0; i < NumSegments; i++)
        {
            float Time1 = (float)i / NumSegments;
            float Time2 = (float)(i + 1) / NumSegments;
            FVector Point1 = SplineComponent->GetLocationAtTime(Time1, ESplineCoordinateSpace::World);
            FVector Point2 = SplineComponent->GetLocationAtTime(Time2, ESplineCoordinateSpace::World);
            DrawDebugLine(GetWorld(), Point1, Point2, FColor::Green, false, -1.0f, 0, 2.0f);
        }

        FVector CurrentPos = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
        DrawDebugSphere(GetWorld(), CurrentPos, 10.0f, 8, FColor::Yellow, false, -1.0f, 0);

#if WITH_EDITOR
        if (bEnableEditorPreview && !GetWorld()->IsGameWorld())
        {
            FVector PreviewPos = SplineComponent->GetLocationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);
            DrawDebugSphere(GetWorld(), PreviewPos, 15.0f, 12, FColor::Cyan, false, -1.0f, 0);
            DrawDebugString(GetWorld(), PreviewPos + FVector(0, 0, 40),
                *FString::Printf(TEXT("预览位置: %.2f"), EditorPreviewPosition),
                nullptr, FColor::Cyan, 0.0f, true);
        }
#endif
    }
}

#if WITH_EDITOR
void ADraggableSplineActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, EditorPreviewPosition))
    {
        UpdatePreviewPosition();
    }

    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, bEnableEditorPreview))
    {
        if (bEnableEditorPreview)
        {
            UpdatePreviewPosition();
        }
        else
        {
            MeshComponent->SetRelativeLocation(InitialMeshLocation);
            MeshComponent->SetRelativeRotation(InitialMeshRotation);
        }
    }
}
#endif

void ADraggableSplineActor::StartDragging()
{
    bCouldPull = false;
    if (inAutoBack)
        StopAutoBack();
    bIsBeingDragged = true;
    MovementAlpha = 0.0f;
    SmoothedMovementDirection = FVector::ZeroVector;
    SetActorTickEnabled(true);

#if WITH_EDITOR
    if (bEnableEditorPreview)
    {
        bEnableEditorPreview = false;
    }
#endif
}

void ADraggableSplineActor::StopDragging()
{
    bIsBeingDragged = false;
    if (!IfHasAutoBack)
    {
        bCouldPull = true;
        SmoothedMovementDirection = FVector::ZeroVector;
        SetActorTickEnabled(false);
    }
    else
    {
        StartAutoBack();
    }
}

void ADraggableSplineActor::StartAutoBack()
{
    if (inAutoBack) return;
    inAutoBack = true;
}

void ADraggableSplineActor::StopAutoBack()
{
    if (!inAutoBack) return;
    inAutoBack = false;
    bCouldPull = true;

    // 确保最终状态完全对齐
    CurrentSplinePosition = DragStartPos;
    TargetLocation = SplineComponent->GetLocationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    MovementAlpha = 1.0f;
    SetMeshPositionAndRotation(TargetLocation, TargetRotation);

    SmoothedMovementDirection = FVector::ZeroVector;
    if (!bIsBeingDragged)
    {
        SetActorTickEnabled(false);
    }
}

void ADraggableSplineActor::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent || !bIsBeingDragged) return;

    if (SmoothedMovementDirection.IsNearlyZero())
    {
        SmoothedMovementDirection = ViewDirection;
    }
    else
    {
        SmoothedMovementDirection = FMath::Lerp(SmoothedMovementDirection, ViewDirection, SmoothingFactor);
    }

    float MovementDelta = CalculateNormalizedMovement(SmoothedMovementDirection);
    if (SingleDirDrag)
        MovementDelta = FMath::Max(MovementDelta, 0.0f);

    if (FMath::Abs(MovementDelta) < 0.001f) return;

    CurrentSplinePosition = FMath::Clamp(CurrentSplinePosition + MovementDelta, 0.0f, 1.0f);

    TargetLocation = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
    MovementAlpha = 0.0f;

    if (bShowDebugVisualization)
    {
        FVector SplineTangent = GetCurrentTangent();
        float ProjectedMovement = FVector::DotProduct(ViewDirection, SplineTangent);
        DrawDebugVisualization(ViewDirection, ProjectedMovement);
    }
}

FVector ADraggableSplineActor::GetCurrentTangent() const
{
    if (SplineComponent)
        return SplineComponent->GetTangentAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World).GetSafeNormal();
    return FVector::ForwardVector;
}

void ADraggableSplineActor::SetStartPosition()
{
    CurrentSplinePosition = DragStartPos;
    TargetLocation = SplineComponent->GetLocationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    SetMeshPositionAndRotation(TargetLocation, TargetRotation);
    MovementAlpha = 1.0f;
}

float ADraggableSplineActor::CalculateNormalizedMovement(const FVector& ViewDirection)
{
    if (!SplineComponent) return 0.0f;
    FVector SplineTangent = GetCurrentTangent();
    float ProjectedMovement = FVector::DotProduct(ViewDirection, SplineTangent);
    if (FMath::Abs(ProjectedMovement) < DeadZone) return 0.0f;
    float ScaledMovement = ProjectedMovement * DragSensitivity;
    return FMath::Clamp(ScaledMovement, -MaxDragSpeed, MaxDragSpeed);
}

void ADraggableSplineActor::DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement)
{
    FVector CurrentLocation = MeshComponent->GetComponentLocation();
    FVector SplineTangent = GetCurrentTangent();

    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + SplineTangent * DebugLineLength, DebugArrowSize, FColor::Green, false, 0.1f, 0, 3.0f);
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ViewDirection * DebugLineLength, DebugArrowSize, FColor::Blue, false, 0.1f, 0, 3.0f);
    FVector ProjectedVector = SplineTangent * ProjectedMovement * DebugLineLength;
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ProjectedVector, DebugArrowSize, FColor::Red, false, 0.1f, 0, 4.0f);
}

void ADraggableSplineActor::UpdateEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
        UpdatePreviewPosition();
#endif
}

void ADraggableSplineActor::ToggleEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        bEnableEditorPreview = !bEnableEditorPreview;
        if (bEnableEditorPreview)
            UpdatePreviewPosition();
        else
        {
            MeshComponent->SetRelativeLocation(InitialMeshLocation);
            MeshComponent->SetRelativeRotation(InitialMeshRotation);
        }
    }
#endif
}

void ADraggableSplineActor::ResetEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        EditorPreviewPosition = 0.0f;
        UpdatePreviewPosition();
    }
#endif
}

void ADraggableSplineActor::UpdatePreviewPosition()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld() && SplineComponent)
    {
        FVector NewLocation = SplineComponent->GetLocationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);
        FRotator NewRotation = SplineComponent->GetRotationAtTime(EditorPreviewPosition, ESplineCoordinateSpace::World);
        SetMeshPositionAndRotation(NewLocation, NewRotation);
        MarkComponentsRenderStateDirty();
    }
#endif
}

void ADraggableSplineActor::SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation)
{
    if (MeshComponent)
    {
        FVector LocalLocation = GetActorTransform().InverseTransformPosition(Location);
        MeshComponent->SetRelativeLocation(LocalLocation);
        if (IfAdjustRotation)
        {
            FRotator LocalRotation = (GetActorTransform().InverseTransformRotation(Rotation.Quaternion())).Rotator();
            MeshComponent->SetRelativeRotation(LocalRotation);
        }
    }
}