#include "Character/OldManCameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/OverlapResult.h"
#include "Character/OldManCharacter.h"

UOldManCameraComponent::UOldManCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 初始化变量
    CameraBoom = nullptr;
    FollowCamera = nullptr;
    CachedOldManCharacter = nullptr;
    bIsShaking = false;
    CurrentCameraMode = TEXT("ThirdPerson");

    // 修改后的变量初始化
    CurrentCameraRotation = FRotator::ZeroRotator;
    DesiredCameraRotation = FRotator::ZeroRotator;
    CurrentLookUpInput = 0.0f;
    CurrentTurnInput = 0.0f;
    SmoothedLookUpInput = 0.0f;
    SmoothedTurnInput = 0.0f;

    // 初始化重力方向
    CurrentGravityDirection = FVector::DownVector;
    DesiredGravityDirection = FVector::DownVector;
}

void UOldManCameraComponent::BeginPlay()
{
    Super::BeginPlay();
    SetThirdPersonMode();
}

void UOldManCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 处理输入平滑
    UpdateInputSmoothing(DeltaTime);

    // 更新相机旋转
    UpdateCameraRotation(DeltaTime);

    // 更新相机位置
    UpdateCameraPosition(DeltaTime);

    // 每帧重置输入值，确保没有输入时值为0
    CurrentLookUpInput = 0.0f;
    CurrentTurnInput = 0.0f;
}

void UOldManCameraComponent::UpdateInputSmoothing(float DeltaTime)
{
    // 平滑插值输入值到0
    SmoothedLookUpInput = FMath::FInterpTo(SmoothedLookUpInput, 0.0f, DeltaTime, InputSmoothingInterpSpeed);
    SmoothedTurnInput = FMath::FInterpTo(SmoothedTurnInput, 0.0f, DeltaTime, InputSmoothingInterpSpeed);
}

void UOldManCameraComponent::UpdateCameraRotation(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

    // 应用当前帧的视角输入（不累加）
    if (FMath::Abs(CurrentTurnInput) > 0.01f || FMath::Abs(CurrentLookUpInput) > 0.01f)
    {
        DesiredCameraRotation.Yaw += CurrentTurnInput * DeltaTime * 60.0f; // 乘以DeltaTime和帧率系数
        DesiredCameraRotation.Pitch += CurrentLookUpInput * DeltaTime * 60.0f;

        // 限制相机俯仰角度
        DesiredCameraRotation.Pitch = FMath::Clamp(DesiredCameraRotation.Pitch, MyCameraData.CameraPitchMin, MyCameraData.CameraPitchMax);
    }

    SmoothCameraRotate(DeltaTime);
}

void UOldManCameraComponent::UpdateCameraRotationInGravity(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

    // 更新重力对齐
    UpdateGravityAlignment(DeltaTime);

    // 获取当前重力上方向
    FVector GravityUp = -CurrentGravityDirection;

    // 获取当前相机旋转
    FQuat CurrentQuat = DesiredCameraRotation.Quaternion();

    // 应用当前帧的视角输入
    if (FMath::Abs(CurrentTurnInput) > 0.01f || FMath::Abs(CurrentLookUpInput) > 0.01f)
    {
        // 修复鼠标输入方向：将上下输入反转
        float YawInput = CurrentTurnInput * DeltaTime * 60.0f;
        float PitchInput = -CurrentLookUpInput * DeltaTime * 60.0f;

        // 将当前旋转分解为重力对齐的局部旋转
        // 1. 首先找到将世界坐标系对齐到重力坐标系的旋转
        FQuat GravityAlignment = FQuat::FindBetweenNormals(FVector::UpVector, GravityUp);

        // 2. 将当前旋转转换为重力局部空间
        FQuat LocalQuat = GravityAlignment.Inverse() * CurrentQuat;
        FRotator LocalRot = LocalQuat.Rotator();

        // 3. 在重力局部空间中应用输入
        LocalRot.Yaw += YawInput;
        LocalRot.Pitch -= PitchInput;

        // 4. 限制俯仰角度
        LocalRot.Pitch = FMath::Clamp(LocalRot.Pitch, MyCameraData.CameraPitchMin, MyCameraData.CameraPitchMax);

        // 5. 转换回世界空间
        FQuat NewLocalQuat = LocalRot.Quaternion();
        FQuat NewWorldQuat = GravityAlignment * NewLocalQuat;

        DesiredCameraRotation = NewWorldQuat.Rotator();

        // 标记有输入时已经处理过旋转
        bHasRecentInput = true;
        bNeedsGravityAlignment = false; // 有输入时不需要重力对齐
    }
    else
    {
        FVector CurrentUp = CurrentQuat.GetUpVector();

        // 检查是否需要重力对齐
        bool bShouldAlignGravity = false;

        // 如果重力方向发生了变化，需要对齐
        static FVector LastGravityDirection = FVector::DownVector;
        bool bGravityChanged = !CurrentGravityDirection.Equals(LastGravityDirection, 0.01f);
        if (bGravityChanged)
        {
            bNeedsGravityAlignment = true;
            LastGravityDirection = CurrentGravityDirection;
        }

        // 如果相机上方向与重力上方向偏差过大，需要对齐
        if (!CurrentUp.Equals(GravityUp, 0.2f) && !bHasRecentInput)
        {
            bNeedsGravityAlignment = true;
        }

        // 执行重力对齐
        if (bNeedsGravityAlignment)
        {
            // 使用更精确的重力对齐方法
            // 保持相机的水平方向，只调整上下方向
            FVector CurrentRight = CurrentQuat.GetRightVector();
            FVector CurrentForward = CurrentQuat.GetForwardVector();

            // 重新计算正确的上方向
            FVector NewUp = -CurrentGravityDirection;

            // 确保前方向与上方向垂直
            FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp).GetSafeNormal();
            if (NewForward.IsNearlyZero())
            {
                // 如果投影结果为零，使用默认前方向
                NewForward = FVector::VectorPlaneProject(FVector::ForwardVector, NewUp).GetSafeNormal();
            }

            // 重新计算右方向
            FVector NewRight = FVector::CrossProduct(NewUp, NewForward).GetSafeNormal();

            // 重新计算前方向，确保正交
            NewForward = FVector::CrossProduct(NewRight, NewUp).GetSafeNormal();

            // 创建新的旋转
            FQuat NewQuat = FRotationMatrix::MakeFromXZ(NewForward, NewUp).ToQuat();

            // 平滑过渡到新的旋转
            if (MyCameraData.bUseCameraSmoothing)
            {
                FQuat ResultQuat = FQuat::Slerp(CurrentQuat, NewQuat, DeltaTime * GravityRotationInterpSpeed);
                DesiredCameraRotation = ResultQuat.Rotator();

                // 检查是否已经对齐完成
                FVector ResultUp = ResultQuat.GetUpVector();
                if (ResultUp.Equals(NewUp, 0.01f))
                {
                    bNeedsGravityAlignment = false;
                }
            }
            else
            {
                DesiredCameraRotation = NewQuat.Rotator();
                bNeedsGravityAlignment = false;
            }
        }

        // 重置输入标记
        bHasRecentInput = false;
    }

    SmoothCameraRotate(DeltaTime);
}

void UOldManCameraComponent::UpdateCameraPosition(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

    // 处理相机震动
    if (bIsShaking)
    {
        ShakeElapsed += DeltaTime;

        if (ShakeElapsed < ShakeDuration)
        {
            float Time = GetWorld()->GetTimeSeconds();
            FVector ShakeOffset = FVector(
                FMath::Sin(Time * 50.0f) * ShakeIntensity,
                FMath::Cos(Time * 45.0f) * ShakeIntensity,
                FMath::Sin(Time * 55.0f) * ShakeIntensity
            );

            FollowCamera->AddLocalOffset(ShakeOffset);
        }
        else
        {
            bIsShaking = false;
        }
    }
}

// 新增：重力对齐更新
void UOldManCameraComponent::UpdateGravityAlignment(float DeltaTime)
{
    if (!CachedOldManCharacter)
        return;

    // 获取角色的重力方向
    if (CachedOldManCharacter)
    {
        DesiredGravityDirection = CachedOldManCharacter->GetGravityDirection();
    }
    else
    {
        DesiredGravityDirection = FVector::DownVector;
    }

    // 平滑过渡重力方向
    CurrentGravityDirection = FMath::VInterpTo(
        CurrentGravityDirection,
        DesiredGravityDirection,
        DeltaTime,
        GravityRotationInterpSpeed
    );
}

void UOldManCameraComponent::SmoothCameraRotate(float DeltaTime)
{
    // 平滑插值相机旋转
    if (MyCameraData.bUseCameraSmoothing)
    {
        CurrentCameraRotation = FMath::RInterpTo(
            CurrentCameraRotation,
            DesiredCameraRotation,
            DeltaTime,
            MyCameraData.CameraRotationInterpSpeed
        );
    }
    else
    {
        CurrentCameraRotation = DesiredCameraRotation;
    }

    // 应用相机旋转到控制器和弹簧臂
    if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->SetControlRotation(CurrentCameraRotation);
    }

    if (CameraBoom)
    {
        CameraBoom->SetWorldRotation(CurrentCameraRotation);
    }
}

void UOldManCameraComponent::InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera, FOldManCameraData CameraData)
{
    MyCameraData = CameraData;

    CameraBoom = InCameraBoom;
    FollowCamera = InFollowCamera;
    CurCameraDistance = MyCameraData.CameraDistance;

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->SocketOffset = MyCameraData.CameraOffset;
        CameraBoom->CameraLagSpeed = MyCameraData.CameraLagSpeed;
        CameraBoom->CameraRotationLagSpeed = MyCameraData.CameraRotationLagSpeed;
    }
}

void UOldManCameraComponent::SetCameraTarget(AOldManCharacter* targetActor)
{
    CachedOldManCharacter = targetActor;
}

void UOldManCameraComponent::SetCameraOffset(const FVector& Offset)
{
    MyCameraData.CameraOffset = Offset;
    if (CameraBoom)
    {
        CameraBoom->SocketOffset = MyCameraData.CameraOffset;
    }
}

void UOldManCameraComponent::SetCameraDistance(float Distance)
{
    CurCameraDistance = Distance;
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CurCameraDistance;
    }
}

void UOldManCameraComponent::SetCameraInput(float rawLookUpInput, float rawTurnInput)
{
    // 直接设置输入值，不累加
    CurrentLookUpInput = rawLookUpInput;
    CurrentTurnInput = rawTurnInput;
}

FRotator UOldManCameraComponent::GetCameraRotation()
{
    return CurrentCameraRotation;
}

void UOldManCameraComponent::ShakeCamera(float Intensity, float Duration)
{
    bIsShaking = true;
    ShakeIntensity = Intensity;
    ShakeDuration = Duration;
    ShakeElapsed = 0.0f;
}

void UOldManCameraComponent::SetThirdPersonMode()
{
    CurrentCameraMode = TEXT("ThirdPerson");
    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }
}

void UOldManCameraComponent::SetPersonInSlopeMode()
{
    CurrentCameraMode = TEXT("FirstPerson");
    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = 0.0f;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = false;
        CameraBoom->bEnableCameraRotationLag = false;
        FollowCamera->bUsePawnControlRotation = true;
    }
}

void UOldManCameraComponent::SetHitchcockLookMode()
{
    CurrentCameraMode = TEXT("FreeLook");
    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }
}

void UOldManCameraComponent::GetActorsInCone(
    FOldManDetectionData DetectionData,
    const FName ValidTag,
    TArray<AActor*>& OutActors,
    TArray<float>& OutDistances,
    TArray<float>& OutAngles
)
{
    OutActors.Empty();
    OutDistances.Empty();
    OutAngles.Empty();

    UWorld* World = GetWorld();
    if (!World || !FollowCamera) return;

    FVector Origin = FollowCamera->GetComponentLocation();
    FVector Direction = FollowCamera->GetForwardVector();

    bool bShowDebug = DetectionData.DebugMode;
    float ConeLength = DetectionData.ConeLength;
    float ConeAngle = DetectionData.ConeAngle;

    FVector NormalizedDirection = Direction.GetSafeNormal();
    float HalfConeAngle = ConeAngle * 0.5f;
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfConeAngle));

    // 可视化：绘制锥形范围
    if (bShowDebug)
    {
        DrawConeVisualization(World, Origin, Direction, ConeLength, ConeAngle, DetectionData.DebugColor, DetectionData.DebugDuration);
    }

    // 如果没有指定通道，使用默认通道
    if (DetectionData.DetectionChannels.Num() == 0)
    {
        DetectionData.DetectionChannels.Add(ECC_Pawn);
    }

    // 对每个碰撞通道进行检测
    for (ECollisionChannel Channel : DetectionData.DetectionChannels)
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner()); // 忽略自身

        FCollisionShape SphereShape = FCollisionShape::MakeSphere(ConeLength);

        bool bHasOverlap = World->OverlapMultiByChannel(
            OverlapResults,
            Origin,
            FQuat::Identity,
            Channel,
            SphereShape,
            QueryParams
        );

        if (!bHasOverlap) continue;

        for (const FOverlapResult& OverlapResult : OverlapResults)
        {
            AActor* Actor = OverlapResult.GetActor();
            if (!Actor) continue;

            // 避免重复添加同一个Actor
            if (OutActors.Contains(Actor)) continue;

            // Tags过滤（如果ValidTag不为None）
            if (ValidTag != NAME_None && Actor->Tags.Find(ValidTag) < 0) continue;

            FVector ActorLocation = Actor->GetActorLocation();
            FVector ToActor = ActorLocation - Origin;
            float Distance = ToActor.Size();

            // 距离检查
            if (Distance > ConeLength || Distance < KINDA_SMALL_NUMBER) continue;

            // 角度检查
            FVector ToActorNormalized = ToActor.GetSafeNormal();
            float DotProduct = FVector::DotProduct(NormalizedDirection, ToActorNormalized);

            if (DotProduct >= CosHalfAngle)
            {
                float Angle = FMath::Acos(DotProduct) * (180.0f / PI);

                //视线遮挡检测
                FHitResult HitResult;
                FCollisionQueryParams LineParams;
                LineParams.AddIgnoredActor(GetOwner());
                LineParams.AddIgnoredActor(Actor);

                if (World->LineTraceSingleByChannel(HitResult, Origin, ActorLocation, Channel, LineParams))
                {
                    // 有物体遮挡，跳过
                    continue;
                }

                OutActors.Add(Actor);
                OutDistances.Add(Distance);
                OutAngles.Add(Angle);

                // 调试绘制：检测到的Actor
                if (bShowDebug)
                {
                    // 根据碰撞通道使用不同颜色
                    FColor ChannelColor = DetectionData.DebugColor;
                    DrawDebugLine(World, Origin, ActorLocation, ChannelColor, false, DetectionData.DebugDuration, 0, 2.0f);
                    DrawDebugPoint(World, ActorLocation, 10.0f, ChannelColor, false, DetectionData.DebugDuration, 0);

                    // 显示Actor信息
                    DrawDebugString(
                        World,
                        ActorLocation + FVector(0, 0, 50),
                        FString::Printf(TEXT("%s\n%.1fm"), *Actor->GetName(), Distance),
                        Actor,
                        FColor::White,
                        DetectionData.DebugDuration
                    );
                }
            }
        }
    }
}

// 增强版可视化绘制
void UOldManCameraComponent::DrawConeVisualization(
    UWorld* World,
    const FVector& Origin,
    const FVector& Direction,
    float ConeLength,
    float ConeAngle,
    FColor Color,
    float Duration
)
{
    if (!World) return;

    // 使用UE内置的锥体绘制（如果可用）
    // 注意：DrawDebugCone在UE4.25+和UE5中可用

    // 计算锥体角度（弧度）
    float HalfConeAngleRad = FMath::DegreesToRadians(ConeAngle * 0.5f);

    // 计算锥体半径
    float ConeRadius = ConeLength * FMath::Tan(HalfConeAngleRad);

    // 获取相机的旋转
    FRotator CameraRotation = Direction.Rotation();

    // 使用DrawDebugCone绘制3D锥体
    DrawDebugCone(
        World,
        Origin,
        Direction,
        ConeLength,
        HalfConeAngleRad,
        HalfConeAngleRad,
        16,        // 分段数
        Color,
        false,
        Duration,
        0,
        2.0f       // 线粗
    );

    // 绘制锥形中心线
    DrawDebugLine(
        World,
        Origin,
        Origin + Direction * ConeLength,
        FColor::Yellow,
        false,
        Duration,
        0,
        1.0f
    );

    // 绘制检测范围信息文本
    if (GEngine)
    {
        FString DebugText = FString::Printf(TEXT("Detection Cone: %.1fm, %.1f°"), ConeLength, ConeAngle);
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color, DebugText);
    }
}

