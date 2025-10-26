#include "Character/OldManCameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/OverlapResult.h"

UOldManCameraComponent::UOldManCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 初始化变量
    CameraBoom = nullptr;
    FollowCamera = nullptr;
    TargetActor = nullptr;
    bIsShaking = false;
    CurrentCameraMode = TEXT("ThirdPerson");

    // 修改后的变量初始化
    CurrentCameraRotation = FRotator::ZeroRotator;
    DesiredCameraRotation = FRotator::ZeroRotator;
    CurrentLookUpInput = 0.0f;
    CurrentTurnInput = 0.0f;
    SmoothedLookUpInput = 0.0f;
    SmoothedTurnInput = 0.0f;
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
    if (!TargetActor || !CameraBoom || !FollowCamera)
        return;

    // 应用当前帧的视角输入（不累加）
    if (FMath::Abs(CurrentTurnInput) > 0.01f || FMath::Abs(CurrentLookUpInput) > 0.01f)
    {
        DesiredCameraRotation.Yaw += CurrentTurnInput * DeltaTime * 60.0f; // 乘以DeltaTime和帧率系数
        DesiredCameraRotation.Pitch += CurrentLookUpInput * DeltaTime * 60.0f;

        // 限制相机俯仰角度
        DesiredCameraRotation.Pitch = FMath::Clamp(DesiredCameraRotation.Pitch, MyCameraData.CameraPitchMin, MyCameraData.CameraPitchMax);
    }

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

void UOldManCameraComponent::UpdateCameraPosition(float DeltaTime)
{
    if (!TargetActor || !CameraBoom || !FollowCamera)
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

void UOldManCameraComponent::SetCameraTarget(AActor* targetActor)
{
    this->TargetActor = targetActor;
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

void UOldManCameraComponent::SetFirstPersonMode()
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

void UOldManCameraComponent::SetFreeLookMode()
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

