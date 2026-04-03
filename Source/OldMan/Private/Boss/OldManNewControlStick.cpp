// OldManNewControlStick.cpp
#include "Boss/OldManNewControlStick.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"

AOldManNewControlStick::AOldManNewControlStick()
{
    PrimaryActorTick.bCanEverTick = true;
    InputDir = FVector::ZeroVector;
    bInAutoBack = false;
    MovementAlpha = 1.0f;
    CurrentOffset = FVector::ZeroVector;
    TargetOffset = FVector::ZeroVector;
    SmoothedMovementDirection = FVector::ZeroVector;

    // 添加根组件
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // 添加碰撞组件（用于射线检测）
    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(RootComponent);
    CollisionComponent->SetBoxExtent(FVector(50, 50, 50));
    CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));

    // 添加拖拽标记，使交互系统能够识别
    Tags.Add("DragableItem");

    // 输出初始化日志
    /*if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("OldManNewControlStick: Initialized"));
    }*/
}

void AOldManNewControlStick::BeginPlay()
{
    Super::BeginPlay();

    //// 输出BeginPlay日志
    //if (GEngine)
    //{
    //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("OldManNewControlStick: BeginPlay"));
    //}

    if (!StickHead)
    {
        UE_LOG(LogTemp, Error, TEXT("Boss_ControlStick: StickHead 未指定！"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, TEXT("ERROR: StickHead 未指定！"));
        }
        return;
    }

    // 记录摇杆头初始位置（中心点）
    InitPos = StickHead->GetActorLocation();

    // 确保摇杆头初始位置正确
    StickHead->SetActorLocation(InitPos);
    CurrentOffset = FVector::ZeroVector;
    TargetOffset = FVector::ZeroVector;

    //// 输出初始化完成日志
    //if (GEngine)
    //{
    //    FString InitPosStr = FString::Printf(TEXT("OldManNewControlStick: InitPos = %s"), *InitPos.ToString());
    //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, InitPosStr);
    //}
}

void AOldManNewControlStick::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 平滑移动：从当前位置向目标位置插值
    if (MovementAlpha < 1.0f)
    {
        MovementAlpha = FMath::Min(MovementAlpha + DeltaTime * 8.0f, 1.0f);
        CurrentOffset = FMath::Lerp(CurrentOffset, TargetOffset, MovementAlpha);
        StickHead->SetActorLocation(InitPos + CurrentOffset);
    }

    // 自动回正逻辑：松开后自动回到中心
    if (bInAutoBack)
    {
        AutoBackTimer += DeltaTime * AutoBackSpeed;
        float Alpha = FMath::Min(AutoBackTimer, 1.0f);
        CurrentOffset = FMath::Lerp(AutoBackStartOffset, FVector::ZeroVector, Alpha);
        StickHead->SetActorLocation(InitPos + CurrentOffset);

        if (Alpha >= 1.0f)
        {
            // 回正完成
            bInAutoBack = false;
            CurrentOffset = FVector::ZeroVector;
            TargetOffset = FVector::ZeroVector;
            MovementAlpha = 1.0f;
            SetActorTickEnabled(false);  // 无拖拽时停止Tick以节省性能
        }
    }

    // 根据当前偏移更新归一化输入方向
    UpdateInputDir();

    // 调试绘制
    if (bShowDebugVisualization)
    {
        DrawDebugVisualization();
    }
}

void AOldManNewControlStick::StartDragging()
{
    //// 输出StartDragging日志
    //if (GEngine)
    //{
    //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("OldManNewControlStick: StartDragging"));
    //}

    if (!StickHead)
    {
        UE_LOG(LogTemp, Error, TEXT("OldManNewControlStick: StartDragging - StickHead is null"));
     /*   if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: StartDragging - StickHead is null"));
        }*/
        return;
    }

    bIsBeingDragged = true;
    bInAutoBack = false;          // 打断自动回正
    SetActorTickEnabled(true);

    // 将当前偏移作为起始点，后续由 UpdateDragPosition 更新目标
    TargetOffset = CurrentOffset;
    MovementAlpha = 1.0f;          // 直接到达当前位置，不产生跳跃
    SmoothedMovementDirection = FVector::ZeroVector;

    //// 输出拖拽开始日志
    //if (GEngine)
    //{
    //    FString DragStartStr = FString::Printf(TEXT("OldManNewControlStick: Drag started at offset = %s"), *CurrentOffset.ToString());
    //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DragStartStr);
    //}
}

void AOldManNewControlStick::StopDragging()
{
    //// 输出StopDragging日志
    //if (GEngine)
    //{
    //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, TEXT("OldManNewControlStick: StopDragging"));
    //}

    if (!StickHead)
    {
        UE_LOG(LogTemp, Error, TEXT("OldManNewControlStick: StopDragging - StickHead is null"));
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("ERROR: StopDragging - StickHead is null"));
        }
        return;
    }

    bIsBeingDragged = false;

    if (bEnableAutoBack)
    {
        // 启动自动回正
        bInAutoBack = true;
        AutoBackStartOffset = CurrentOffset;
        AutoBackTimer = 0.0f;
        
        // 输出自动回正日志
        if (GEngine)
        {
            FString AutoBackStr = FString::Printf(TEXT("OldManNewControlStick: Auto back started from offset = %s"), *CurrentOffset.ToString());
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, AutoBackStr);
        }
    }
    else
    {
        // 无自动回正：保持当前位置，停止Tick
        SetActorTickEnabled(false);
        
        //// 输出停止拖拽日志
        //if (GEngine)
        //{
        //    FString StopDragStr = FString::Printf(TEXT("OldManNewControlStick: Drag stopped at offset = %s"), *CurrentOffset.ToString());
        //    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, StopDragStr);
        //}
    }
}

void AOldManNewControlStick::UpdateDragPosition(const FVector& WorldPosition)
{
    if (!bIsBeingDragged || !StickHead) return;

    // 计算从当前位置到目标位置的相对偏移
    FVector CurrentPos = InitPos + CurrentOffset;
    FVector DesiredPos = WorldPosition;
    DesiredPos.Z = CurrentPos.Z;

    // 计算相对偏移量
    FVector RelativeOffset = DesiredPos - CurrentPos;

    // 计算新的目标偏移
    FVector DesiredOffset = CurrentOffset + RelativeOffset;

    // 限制偏移量不超过最大半径
    ClampOffset(DesiredOffset);

    // 应用半球面效果
    if (bEnableHemisphereMode)
    {
        DesiredOffset = CalculateHemisphereOffset(DesiredOffset);
    }

    // 应用灵敏度
    DesiredOffset *= DragSensitivity;

    // 限制偏移量不超过最大半径（再次限制，因为应用了灵敏度）
    ClampOffset(DesiredOffset);

    // 更新目标偏移
    TargetOffset = DesiredOffset;

    // 重置插值进度，开始平滑移动
    MovementAlpha = 0.0f;
}

void AOldManNewControlStick::ResetToCenter()
{
    if (!StickHead) return;

    TargetOffset = FVector::ZeroVector;
    MovementAlpha = 0.0f;
    bIsBeingDragged = false;
    bInAutoBack = false;
    SetActorTickEnabled(true);  // 开启Tick以完成平滑移动
}

FVector AOldManNewControlStick::GetWorldDirection()
{
    return InputDir;
}

FVector AOldManNewControlStick::GetCurrentOffset() const
{
    return CurrentOffset;
}

void AOldManNewControlStick::UpdateInputDir()
{
    float Length = CurrentOffset.Size();
    if (Length > DeadZone)
    {
        // 归一化方向，忽略 Z 轴（仅水平面）
        InputDir = CurrentOffset / MaxRadius;
        InputDir.Z = 0.0f;
    }
    else
    {
        InputDir = FVector::ZeroVector;
    }
    TargetHead->ApplyInput(InputDir);
}

void AOldManNewControlStick::ClampOffset(FVector& Offset) const
{
    float Length = Offset.Size();
    if (Length > MaxRadius)
    {
        Offset = Offset.GetSafeNormal() * MaxRadius;
    }
}

void AOldManNewControlStick::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
    // 输出HandleMouseData日志
    //if (GEngine)
    //{
    //    FString MouseDataStr = FString::Printf(TEXT("OldManNewControlStick: HandleMouseData - ViewDirection: %s, Intensity: %f"), *ViewDirection.ToString(), Intensity);
    //    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, MouseDataStr);
    //}

    if (!bIsBeingDragged || !StickHead)
    {
        if (!bIsBeingDragged && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("HandleMouseData: Not being dragged"));
        }
        if (!StickHead && GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("HandleMouseData: StickHead is null"));
        }
        return;
    }

    // 平滑移动方向
    if (SmoothedMovementDirection.IsNearlyZero())
    {
        SmoothedMovementDirection = ViewDirection;
    }
    else
    {
        SmoothedMovementDirection = FMath::Lerp(SmoothedMovementDirection, ViewDirection, SmoothingFactor);
    }

    // 计算目标位置（基于当前位置的相对移动）
    FVector CurrentPos = InitPos + CurrentOffset;
    FVector DesiredPos = CurrentPos + SmoothedMovementDirection * MaxRadius * Intensity;
    
    // 限制在平面上（基础位置）
    FVector PlanarPos = DesiredPos;
    PlanarPos.Z = CurrentPos.Z;
    
    // 计算偏移
    FVector DesiredOffset = PlanarPos - InitPos;
    
    // 限制范围
    ClampOffset(DesiredOffset);
    
    // 应用半球面效果
    if (bEnableHemisphereMode)
    {
        DesiredOffset = CalculateHemisphereOffset(DesiredOffset);
    }
    
    // 更新目标偏移
    TargetOffset = DesiredOffset;
    
    // 重置插值进度
    MovementAlpha = 0.0f;

    //// 输出目标偏移日志
    //if (GEngine)
    //{
    //    FString TargetOffsetStr = FString::Printf(TEXT("OldManNewControlStick: TargetOffset = %s"), *TargetOffset.ToString());
    //    GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TargetOffsetStr);
    //}
}

FVector AOldManNewControlStick::CalculateHemisphereOffset(const FVector& PlanarOffset) const
{
    float PlanarLength = PlanarOffset.Size();
    float Height = 0.0f;
    
    if (bEnableHemisphereMode && PlanarLength > 0)
    {
        float NormalizedLength = PlanarLength / MaxRadius;
        Height = (1.0f - (NormalizedLength * NormalizedLength)) * MaxRadius * HemisphereHeightFactor;
    }
    
    FVector HemisphereOffset = PlanarOffset;
    HemisphereOffset.Z = Height;
    return HemisphereOffset;
}

void AOldManNewControlStick::DrawDebugVisualization() const
{
    if (!GetWorld()) return;

    // 绘制碰撞组件范围
    if (CollisionComponent)
    {
        FVector BoxExtent = CollisionComponent->GetUnscaledBoxExtent();
        FVector BoxLocation = GetActorLocation();
        DrawDebugBox(GetWorld(), BoxLocation, BoxExtent, FColor::Blue, false, -1.0f, 0, 2.0f);
    }

    // 绘制摇杆活动范围圆（分段绘制线条）
    const int32 Segments = DebugCircleSegments;
    const float Step = 2 * PI / Segments;
    for (int32 i = 0; i < Segments; ++i)
    {
        float Angle1 = i * Step;
        float Angle2 = (i + 1) * Step;
        FVector Point1 = InitPos + FVector(FMath::Cos(Angle1) * MaxRadius, FMath::Sin(Angle1) * MaxRadius, 0);
        FVector Point2 = InitPos + FVector(FMath::Cos(Angle2) * MaxRadius, FMath::Sin(Angle2) * MaxRadius, 0);
        DrawDebugLine(GetWorld(), Point1, Point2, DebugCircleColor, false, -1.0f, 0, 2.0f);
    }

    // 绘制半球面网格（可选）
    if (bEnableHemisphereMode)
    {
        const int32 HemisphereSegments = 16;
        const float HemisphereStep = 2 * PI / HemisphereSegments;
        
        for (int32 i = 0; i < HemisphereSegments; ++i)
        {
            for (int32 j = 0; j < HemisphereSegments / 2; ++j)
            {
                float Angle1 = i * HemisphereStep;
                float Angle2 = (i + 1) * HemisphereStep;
                float Elevation1 = j * HemisphereStep / 2;
                float Elevation2 = (j + 1) * HemisphereStep / 2;
                
                FVector Point1 = InitPos + FVector(
                    FMath::Cos(Angle1) * FMath::Sin(Elevation1) * MaxRadius,
                    FMath::Sin(Angle1) * FMath::Sin(Elevation1) * MaxRadius,
                    FMath::Cos(Elevation1) * MaxRadius * HemisphereHeightFactor
                );
                
                FVector Point2 = InitPos + FVector(
                    FMath::Cos(Angle2) * FMath::Sin(Elevation1) * MaxRadius,
                    FMath::Sin(Angle2) * FMath::Sin(Elevation1) * MaxRadius,
                    FMath::Cos(Elevation1) * MaxRadius * HemisphereHeightFactor
                );
                
                FVector Point3 = InitPos + FVector(
                    FMath::Cos(Angle1) * FMath::Sin(Elevation2) * MaxRadius,
                    FMath::Sin(Angle1) * FMath::Sin(Elevation2) * MaxRadius,
                    FMath::Cos(Elevation2) * MaxRadius * HemisphereHeightFactor
                );
                
                DrawDebugLine(GetWorld(), Point1, Point2, DebugHemisphereColor, false, -1.0f, 0, 1.0f);
                DrawDebugLine(GetWorld(), Point1, Point3, DebugHemisphereColor, false, -1.0f, 0, 1.0f);
            }
        }
    }

    // 绘制当前摇杆头位置（黄色球体）
    DrawDebugSphere(GetWorld(), StickHead->GetActorLocation(), 10.0f, 8, FColor::Yellow, false, -1.0f, 0);

    // 绘制方向指示线（红色箭头）
    if (!InputDir.IsNearlyZero())
    {
        FVector DirEnd = InitPos + InputDir * MaxRadius;
        DrawDebugDirectionalArrow(GetWorld(), InitPos, DirEnd, 20.0f, FColor::Red, false, -1.0f, 0, 3.0f);
    }
}