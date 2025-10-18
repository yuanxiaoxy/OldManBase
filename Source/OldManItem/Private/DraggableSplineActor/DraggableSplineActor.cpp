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

    Tags.Add(DragableItem);
}

void ADraggableSplineActor::BeginPlay()
{
    Super::BeginPlay();

    // 存储网格体的初始位置和旋转
    InitialMeshLocation = MeshComponent->GetRelativeLocation();
    InitialMeshRotation = MeshComponent->GetRelativeRotation();

    TargetLocation = MeshComponent->GetComponentLocation();
    TargetRotation = MeshComponent->GetComponentRotation();
}

void ADraggableSplineActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

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
    bIsBeingDragged = true;
    MovementAlpha = 0.0f;

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
    bIsBeingDragged = false;
}

void ADraggableSplineActor::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
    MoveBasedOnViewDirection(ViewDirection, Intensity);
}

FVector ADraggableSplineActor::GetCurrentTangent() const
{
    if (SplineComponent)
    {
        return SplineComponent->GetTangentAtTime(CurrentSplinePosition, ESplineCoordinateSpace::World).GetSafeNormal();
    }
    return FVector::ForwardVector;
}

float ADraggableSplineActor::CalculateNormalizedMovement(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent) return 0.0f;

    // 获取样条线在当前点的切线方向（路径方向）
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

    // 应用非线性响应曲线，改善手感
    // 小幅移动更敏感，大幅移动更平滑
    float ResponsiveMovement = FMath::Sign(ClampedMovement) * FMath::Pow(FMath::Abs(ClampedMovement), 0.8f);

    return ResponsiveMovement;
}

void ADraggableSplineActor::MoveBasedOnViewDirection(const FVector& ViewDirection, float Intensity)
{
    if (!SplineComponent || !bIsBeingDragged) return;

    // 计算归一化移动量
    float MovementDelta = CalculateNormalizedMovement(ViewDirection, Intensity);

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

        // 显示移动信息
        if (GEngine)
        {
            FString DebugText = FString::Printf(
                TEXT("投影值: %.2f\n位置: %.2f\n移动量: %.4f"),
                ProjectedMovement, CurrentSplinePosition, MovementDelta
            );
            GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::White, DebugText);
        }
    }
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

    // 4. 绘制样条线起点和终点
    FVector StartLocation = SplineComponent->GetLocationAtTime(0.0f, ESplineCoordinateSpace::World);
    FVector EndLocation = SplineComponent->GetLocationAtTime(1.0f, ESplineCoordinateSpace::World);

    DrawDebugSphere(GetWorld(), StartLocation, 15.0f, 12, FColor::Green, false, 0.1f, 0);
    DrawDebugSphere(GetWorld(), EndLocation, 15.0f, 12, FColor::Red, false, 0.1f, 0);

    // 5. 绘制样条线方向标签
    DrawDebugString(GetWorld(), StartLocation + FVector(0, 0, 30), TEXT("StartPoint"), nullptr, FColor::Green, 0.1f, false);
    DrawDebugString(GetWorld(), EndLocation + FVector(0, 0, 30), TEXT("EndPoint"), nullptr, FColor::Red, 0.1f, false);

    // 6. 绘制当前点切线方向标签
    DrawDebugString(GetWorld(), CurrentLocation + FVector(0, 0, 50),
        *FString::Printf(TEXT("切线: (%.1f, %.1f, %.1f)"), SplineTangent.X, SplineTangent.Y, SplineTangent.Z),
        nullptr, FColor::Green, 0.1f, false);
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
        FRotator LocalRotation = (GetActorTransform().InverseTransformRotation(Rotation.Quaternion())).Rotator();

        MeshComponent->SetRelativeLocation(LocalLocation);
        MeshComponent->SetRelativeRotation(LocalRotation);
    }
}