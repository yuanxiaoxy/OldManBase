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

    //自动回弹时所需数值
    LerpStartPosition = CurrentSplinePosition;
    AutoBackTimer = 0.0f;

    Tags.Add("DragableItem");
}

void ADraggableSplineActor::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    // 存储网格体的初始位置和旋转
    InitialMeshLocation = SplineComponent->GetLocationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    InitialMeshRotation = SplineComponent->GetRotationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    SetStartPosition();

    TargetLocation = MeshComponent->GetComponentLocation();
    TargetRotation = MeshComponent->GetComponentRotation();
}

void ADraggableSplineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //自动归位计算
    if (inAutoBack && IfHasAutoBack)
    {
        MovementAlpha = 0.0f;

        //判断是否匀速变化
        if (IfAutoBackUniformSpeed)//匀速
        {
            //更新t
            AutoBackTimer = FMath::Min(AutoBackTimer + DeltaTime / AutoBackRateOrTime, 1.0f);
            // 更新位置
            CurrentSplinePosition = FMath::Lerp(LerpStartPosition, DragStartPos, AutoBackTimer);
        }
        else//先快后慢
        {
            //更新t
            //AutoBackTimer = FMath::Min(DeltaTime * AutoBackRateOrTime, 1.0f);
            AutoBackTimer = FMath::Min(DeltaTime * AutoBackRateOrTime, 1.0f);
            //更新位置
            CurrentSplinePosition = FMath::Lerp(CurrentSplinePosition, DragStartPos, AutoBackTimer);
        }
        
        // 计算新位置和旋转
        TargetLocation = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
        TargetRotation = SplineComponent->GetRotationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);

        if (FVector::Dist(MeshComponent->GetComponentLocation(), InitialMeshLocation) < 0.001f)
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

    // 持续绘制样条线调试（即使不在拖动状态）
    if (bShowDebugVisualization && SplineComponent)
    {
        // 绘制样条线路径
        const int32 NumSegments = 50;
        for (int32 i = 0; i < NumSegments; i++)
        {
            float Time1 = (float)i / NumSegments;
            float Time2 = (float)(i + 1) / NumSegments;

            FVector Point1 = SplineComponent->GetLocationAtTime(Time1, ESplineCoordinateSpace::World);
            FVector Point2 = SplineComponent->GetLocationAtTime(Time2, ESplineCoordinateSpace::World);

            DrawDebugLine(GetWorld(), Point1, Point2, FColor::Green, false, -1.0f, 0, 2.0f);
        }

        // 绘制当前位置标记
        FVector CurrentPos = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
        DrawDebugSphere(GetWorld(), CurrentPos, 10.0f, 8, FColor::Yellow, false, -1.0f, 0);

        // 绘制编辑器预览位置标记（如果启用）
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

    // 当编辑器预览位置改变时，立即更新
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, EditorPreviewPosition))
    {
        UpdatePreviewPosition();
    }

    // 当启用/禁用预览时，重置位置
    if (PropertyName == GET_MEMBER_NAME_CHECKED(ADraggableSplineActor, bEnableEditorPreview))
    {
        if (bEnableEditorPreview)
        {
            UpdatePreviewPosition();
        }
        else
        {
            // 恢复原始位置 - 重置网格体位置
            MeshComponent->SetRelativeLocation(InitialMeshLocation);
            MeshComponent->SetRelativeRotation(InitialMeshRotation);
        }
    }
}
#endif

void ADraggableSplineActor::StartDragging()
{
    //取消自动回弹
    inAutoBack = false;
    bIsBeingDragged = true;
    MovementAlpha = 0.0f;
    SmoothedMovementDirection = FVector::ZeroVector;
    SetActorTickEnabled(true);

    // 如果启用了编辑器预览，暂时禁用它
#if WITH_EDITOR
    if (bEnableEditorPreview)
    {
        bEnableEditorPreview = false;
    }
#endif
}

void ADraggableSplineActor::StopDragging()
{
    if (!IfHasAutoBack)
    {
        bIsBeingDragged = false;
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
    //自动回弹相关
    LerpStartPosition = CurrentSplinePosition;
    AutoBackTimer = 0.0f;

    inAutoBack = true;
}

void ADraggableSplineActor::StopAutoBack()
{
    //自动回弹相关
    inAutoBack = false;
    CurrentSplinePosition = DragStartPos;
    SmoothedMovementDirection = FVector::ZeroVector;
    SetActorTickEnabled(false);
}

void ADraggableSplineActor::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent || !bIsBeingDragged) return;

    // 平滑移动方向
    if (SmoothedMovementDirection.IsNearlyZero())
    {
        SmoothedMovementDirection = ViewDirection;
    }
    else
    {
        SmoothedMovementDirection = FMath::Lerp(SmoothedMovementDirection, ViewDirection, SmoothingFactor);
    }

    // 计算归一化移动量
    float MovementDelta = CalculateNormalizedMovement(SmoothedMovementDirection);

    // 如果移动量很小，忽略
    if (FMath::Abs(MovementDelta) < 0.001f) return;

    // 更新位置
    CurrentSplinePosition = FMath::Clamp(CurrentSplinePosition + MovementDelta, 0.0f, 1.0f);

    // 计算新位置和旋转
    TargetLocation = SplineComponent->GetLocationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World);

    MovementAlpha = 0.0f;

    // 绘制调试可视化
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
    {
        return SplineComponent->GetTangentAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World).GetSafeNormal();
    }
    return FVector::ForwardVector;
}

void ADraggableSplineActor::SetStartPosition()
{
    CurrentSplinePosition = DragStartPos;
    // 计算初始位置和旋转
    TargetLocation = SplineComponent->GetLocationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    TargetRotation = SplineComponent->GetRotationAtTime(DragStartPos, ESplineCoordinateSpace::World);
    SetMeshPositionAndRotation(TargetLocation, TargetRotation);
}

float ADraggableSplineActor::CalculateNormalizedMovement(const FVector& ViewDirection)
{
    if (!SplineComponent) return 0.0f;

    // 获取样条线在当前点的切线方向
    FVector SplineTangent = GetCurrentTangent();

    // 将视角移动方向投影到样条线切线上
    float ProjectedMovement = FVector::DotProduct(ViewDirection, SplineTangent);

    // 应用死区 - 小幅度移动不响应
    if (FMath::Abs(ProjectedMovement) < DeadZone)
    {
        return 0.0f;
    }

    // 应用灵敏度
    float ScaledMovement = ProjectedMovement * DragSensitivity;

    // 限制最大速度
    float ClampedMovement = FMath::Clamp(ScaledMovement, -MaxDragSpeed, MaxDragSpeed);

    return ClampedMovement;
}

void ADraggableSplineActor::DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement)
{
    FVector CurrentLocation = MeshComponent->GetComponentLocation();
    FVector SplineTangent = GetCurrentTangent();

    // 1. 绘制样条线切线方向（绿色）
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + SplineTangent * DebugLineLength, DebugArrowSize, FColor::Green, false, 0.1f, 0, 3.0f);

    // 2. 绘制视角移动方向（蓝色）
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ViewDirection * DebugLineLength, DebugArrowSize, FColor::Blue, false, 0.1f, 0, 3.0f);

    // 3. 绘制投影结果（红色）
    FVector ProjectedVector = SplineTangent * ProjectedMovement * DebugLineLength;
    DrawDebugDirectionalArrow(GetWorld(), CurrentLocation,
        CurrentLocation + ProjectedVector, DebugArrowSize, FColor::Red, false, 0.1f, 0, 4.0f);
}

// 编辑器预览函数实现
void ADraggableSplineActor::UpdateEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        UpdatePreviewPosition();
    }
#endif
}

void ADraggableSplineActor::ToggleEditorPreview()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        bEnableEditorPreview = !bEnableEditorPreview;
        if (bEnableEditorPreview)
        {
            UpdatePreviewPosition();
        }
        else
        {
            // 重置网格体位置
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

        // 在编辑器中标记为需要重绘
        MarkComponentsRenderStateDirty();
    }
#endif
}

void ADraggableSplineActor::SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation)
{
    if (MeshComponent)
    {
        // 将世界坐标转换为相对于Actor的局部坐标
        FVector LocalLocation = GetActorTransform().InverseTransformPosition(Location);
        MeshComponent->SetRelativeLocation(LocalLocation);

        if (IfAdjustRotation)
        {
            FRotator LocalRotation = (GetActorTransform().InverseTransformRotation(Rotation.Quaternion())).Rotator();
            MeshComponent->SetRelativeRotation(LocalRotation);
        }
    }
}