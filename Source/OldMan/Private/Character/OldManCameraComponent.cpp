#include "Character/OldManCameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/OverlapResult.h"
#include "Character/OldManCharacter.h"
#include "Curves/CurveFloat.h"

UOldManCameraComponent::UOldManCameraComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // 创建TimelineComponent
    FadeHitchcockZoomTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("HitchcockZoomTimeline"));

    // 初始化变量
    CameraBoom = nullptr;
    FollowCamera = nullptr;
    CachedOldManCharacter = nullptr;
    bIsShaking = false;
    CurrentCameraMode = ECameraMode::ThirdPersonMode;
    bIsMouseCursorMode = false;

    CurrentCameraRotation = FRotator::ZeroRotator;
    DesiredCameraRotation = FRotator::ZeroRotator;
    CurrentLookUpInput = 0.0f;
    CurrentTurnInput = 0.0f;
    SmoothedLookUpInput = 0.0f;
    SmoothedTurnInput = 0.0f;
    InputSmoothingInterpSpeed = 10.0f;

    CurrentGravityDirection = FVector::DownVector;
    DesiredGravityDirection = FVector::DownVector;
    GravityRotationInterpSpeed = 5.0f;

    FadeInHitchcockTimeLineFloat.BindUFunction(this, FName("FadeInHitchcock"));
    FadeOutHitchcockTimeLineFloat.BindUFunction(this, FName("FadeOutHitchcock"));
    OnHitchcockTimelineFinished.BindUFunction(this, FName("OnHitchcockTimelineFinishedCallback"));
}

void UOldManCameraComponent::InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera, FOldManCameraData CameraData)
{
    MyCameraData = CameraData;

    CameraBoom = InCameraBoom;
    FollowCamera = InFollowCamera;
    OriginalCameraDistance = MyCameraData.CameraDistance;
    OriginalCameraFOV = MyCameraData.CameraFOV;
    CurCameraDistance = MyCameraData.CameraDistance;
    CurCameraFOV = MyCameraData.CameraFOV;
    InputSmoothingInterpSpeed = MyCameraData.InputSmoothingInterpSpeed;
    GravityRotationInterpSpeed = MyCameraData.GravityRotationInterpSpeed;
    CameraFrameRate = MyCameraData.CameraFrameRate;

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->SocketOffset = MyCameraData.CameraOffset;
        CameraBoom->CameraLagSpeed = MyCameraData.CameraLagSpeed;
        CameraBoom->CameraRotationLagSpeed = MyCameraData.CameraRotationLagSpeed;
    }
}

void UOldManCameraComponent::BeginPlay()
{
    Super::BeginPlay();

    if (FadeHitchcockZoomTimeline)
    {
        FadeHitchcockZoomTimeline->SetTimelineFinishedFunc(OnHitchcockTimelineFinished);
    }

    SetThirdPersonMode();
}

void UOldManCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateCamera(DeltaTime);

    CurrentLookUpInput = 0.0f;
    CurrentTurnInput = 0.0f;
}

void UOldManCameraComponent::UpdateCamera(float DeltaTime)
{
    switch (CurrentCameraMode)
    {
    case ECameraMode::ThirdPersonMode:
        UpdateInputSmoothing(DeltaTime);
        UpdateCameraRotation(DeltaTime);
        UpdateCameraPosition(DeltaTime);
        break;
    case ECameraMode::ControlByGravityMode:
        UpdateInputSmoothing(DeltaTime);
        UpdateCameraRotationInGravity(DeltaTime);
        UpdateCameraPosition(DeltaTime);
        break;
    case ECameraMode::MouseCursorMode:
        UpdateCameraPosition(DeltaTime);
        break;
    default:
        break;
    }
}

void UOldManCameraComponent::UpdateInputSmoothing(float DeltaTime)
{
    SmoothedLookUpInput = FMath::FInterpTo(SmoothedLookUpInput, 0.0f, DeltaTime, InputSmoothingInterpSpeed);
    SmoothedTurnInput = FMath::FInterpTo(SmoothedTurnInput, 0.0f, DeltaTime, InputSmoothingInterpSpeed);
}

void UOldManCameraComponent::UpdateCameraRotation(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

    if (CachedOldManCharacter->IsCameraAnimationPlaying())
    {
        return;
    }

    // 直接应用输入增量（不再乘以DeltaTime * CameraFrameRate）
    if (FMath::Abs(CurrentTurnInput) > 0.01f || FMath::Abs(CurrentLookUpInput) > 0.01f)
    {
        DesiredCameraRotation.Yaw += CurrentTurnInput;
        DesiredCameraRotation.Pitch += CurrentLookUpInput;

        DesiredCameraRotation.Pitch = FMath::Clamp(DesiredCameraRotation.Pitch, MyCameraData.CameraPitchMin, MyCameraData.CameraPitchMax);
    }

    SmoothCameraRotate(DeltaTime);
}

void UOldManCameraComponent::UpdateCameraRotationInGravity(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

    UpdateGravityAlignment(DeltaTime);

    if (CachedOldManCharacter->IsCameraAnimationPlaying())
    {
        return;
    }

    FVector GravityUp = -CurrentGravityDirection;
    FQuat CurrentQuat = DesiredCameraRotation.Quaternion();

    if (FMath::Abs(CurrentTurnInput) > 0.01f || FMath::Abs(CurrentLookUpInput) > 0.01f)
    {
        // 直接使用输入增量（不再乘以DeltaTime * 60.0f）
        float YawInput = CurrentTurnInput;
        float PitchInput = -CurrentLookUpInput; // 上下反转

        FQuat GravityAlignment = FQuat::FindBetweenNormals(FVector::UpVector, GravityUp);
        FQuat LocalQuat = GravityAlignment.Inverse() * CurrentQuat;
        FRotator LocalRot = LocalQuat.Rotator();

        LocalRot.Yaw += YawInput;
        LocalRot.Pitch -= PitchInput;

        LocalRot.Pitch = FMath::Clamp(LocalRot.Pitch, MyCameraData.CameraPitchMin, MyCameraData.CameraPitchMax);

        FQuat NewLocalQuat = LocalRot.Quaternion();
        FQuat NewWorldQuat = GravityAlignment * NewLocalQuat;

        DesiredCameraRotation = NewWorldQuat.Rotator();

        bHasRecentInput = true;
        bNeedsGravityAlignment = false;
    }
    else
    {
        FVector CurrentUp = CurrentQuat.GetUpVector();

        static FVector LastGravityDirection = FVector::DownVector;
        bool bGravityChanged = !CurrentGravityDirection.Equals(LastGravityDirection, 0.01f);
        if (bGravityChanged)
        {
            bNeedsGravityAlignment = true;
            LastGravityDirection = CurrentGravityDirection;
        }

        if (!CurrentUp.Equals(GravityUp, 0.2f) && !bHasRecentInput)
        {
            bNeedsGravityAlignment = true;
        }

        if (bNeedsGravityAlignment)
        {
            FVector CurrentRight = CurrentQuat.GetRightVector();
            FVector CurrentForward = CurrentQuat.GetForwardVector();

            FVector NewUp = -CurrentGravityDirection;
            FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp).GetSafeNormal();
            if (NewForward.IsNearlyZero())
            {
                NewForward = FVector::VectorPlaneProject(FVector::ForwardVector, NewUp).GetSafeNormal();
            }

            FVector NewRight = FVector::CrossProduct(NewUp, NewForward).GetSafeNormal();
            NewForward = FVector::CrossProduct(NewRight, NewUp).GetSafeNormal();

            FQuat NewQuat = FRotationMatrix::MakeFromXZ(NewForward, NewUp).ToQuat();

            if (MyCameraData.bUseCameraSmoothing)
            {
                FQuat ResultQuat = FQuat::Slerp(CurrentQuat, NewQuat, DeltaTime * GravityRotationInterpSpeed);
                DesiredCameraRotation = ResultQuat.Rotator();

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

        bHasRecentInput = false;
    }

    SmoothCameraRotate(DeltaTime);
}

void UOldManCameraComponent::UpdateCameraPosition(float DeltaTime)
{
    if (!CachedOldManCharacter || !CameraBoom || !FollowCamera)
        return;

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

void UOldManCameraComponent::UpdateGravityAlignment(float DeltaTime)
{
    if (!CachedOldManCharacter)
        return;

    if (CachedOldManCharacter)
    {
        DesiredGravityDirection = CachedOldManCharacter->GetGravityDirection();
    }
    else
    {
        DesiredGravityDirection = FVector::DownVector;
    }

    CurrentGravityDirection = FMath::VInterpTo(
        CurrentGravityDirection,
        DesiredGravityDirection,
        DeltaTime,
        GravityRotationInterpSpeed
    );
}

void UOldManCameraComponent::SmoothCameraRotate(float DeltaTime)
{
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

    if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        PlayerController->SetControlRotation(CurrentCameraRotation);
    }

    if (CameraBoom)
    {
        CameraBoom->SetWorldRotation(CurrentCameraRotation);
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
    if (CameraBoom && NotToControlCameraDisState)
    {
        CameraBoom->TargetArmLength = CurCameraDistance;
    }
}

void UOldManCameraComponent::SetCameraFOV(float NewFOV)
{
    CurCameraFOV = NewFOV;
    if (FollowCamera)
    {
        FollowCamera->SetFieldOfView(CurCameraFOV);
    }
}

void UOldManCameraComponent::SetCameraInput(float rawLookUpInput, float rawTurnInput)
{
    CurrentLookUpInput = rawLookUpInput;
    CurrentTurnInput = rawTurnInput;
}

FVector UOldManCameraComponent::GetCameraLocation() const
{
    if (FollowCamera)
    {
        return FollowCamera->GetComponentLocation();
    }
    return FVector::ZeroVector;
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
    CurrentCameraMode = ECameraMode::ThirdPersonMode;
    bIsMouseCursorMode = false;

    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }

    if (!CachedOldManCharacter || !CachedOldManCharacter->IsCameraAnimationPlaying())
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (PlayerController)
        {
            PlayerController->SetInputMode(FInputModeGameOnly());
            PlayerController->bShowMouseCursor = false;
        }
    }
}

void UOldManCameraComponent::SetPersonInSlopeMode()
{
    CurrentCameraMode = ECameraMode::ControlByGravityMode;
    bIsMouseCursorMode = false;

    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }

    if (!CachedOldManCharacter || !CachedOldManCharacter->IsCameraAnimationPlaying())
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (PlayerController)
        {
            PlayerController->SetInputMode(FInputModeGameOnly());
            PlayerController->bShowMouseCursor = false;
        }
    }
}

void UOldManCameraComponent::SetMouseCursorMode()
{
    CurrentCameraMode = ECameraMode::MouseCursorMode;
    bIsMouseCursorMode = true;

    if (CameraBoom && FollowCamera)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->bUsePawnControlRotation = false;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }

    if (!CachedOldManCharacter || !CachedOldManCharacter->IsCameraAnimationPlaying())
    {
        APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
        if (PlayerController)
        {
            FInputModeGameAndUI InputMode;
            InputMode.SetHideCursorDuringCapture(false);
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PlayerController->SetInputMode(InputMode);
            PlayerController->bShowMouseCursor = true;
        }
    }
}

void UOldManCameraComponent::FadeInHitchcock(float Alpha)
{
    if (!FollowCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("FadeInHitchcock: FollowCamera is null"));
        return;
    }

    float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

    CurCameraFOV = FMath::Lerp(OriginalCameraFOV, TargetCameraFOV, SmoothAlpha);
    FollowCamera->SetFieldOfView(CurCameraFOV);

    CurCameraDistance = FMath::Lerp(OriginalCameraDistance, TargetCameraDistance, SmoothAlpha);
    SetCameraDistance(CurCameraDistance);
}

void UOldManCameraComponent::FadeOutHitchcock(float Alpha)
{
    if (!FollowCamera)
    {
        UE_LOG(LogTemp, Warning, TEXT("FadeOutHitchcock: FollowCamera is null"));
        return;
    }

    float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

    CurCameraFOV = FMath::Lerp(TargetCameraFOV, OriginalCameraFOV, SmoothAlpha);
    FollowCamera->SetFieldOfView(CurCameraFOV);

    CurCameraDistance = FMath::Lerp(TargetCameraDistance, OriginalCameraDistance, SmoothAlpha);
    SetCameraDistance(CurCameraDistance);
}

void UOldManCameraComponent::SetCameraInHitchcock(float TargetFOV, float TargetDistance)
{
    if (FMath::IsNearlyEqual(CurCameraFOV, TargetCameraFOV, 0.1f) &&
        FMath::IsNearlyEqual(CurCameraDistance, TargetCameraDistance, 0.1f))
    {
        UE_LOG(LogTemp, Warning, TEXT("SetCameraInHitchcock: Already at target values"));
        return;
    }

    NotToControlCameraDisState = true;
    TargetCameraFOV = TargetFOV;
    TargetCameraDistance = TargetDistance;

    if (FadeHitchcockZoomTimeline && CachedOldManCharacter && CachedOldManCharacter->CharacterAttributes)
    {
        FadeHitchcockZoomTimeline->Stop();
        FadeHitchcockZoomTimeline->SetPlaybackPosition(0.0f, false);
        FadeHitchcockZoomTimeline->SetFloatCurve(nullptr, FName("FadeInHitchcock"));

        auto fadeInCurve = CachedOldManCharacter->CharacterAttributes->OldManCameraHitchcockData.FadeInHitchcockCurve;
        if (fadeInCurve)
        {
            FadeHitchcockZoomTimeline->AddInterpFloat(fadeInCurve, FadeInHitchcockTimeLineFloat, FName("FadeInHitchcock"));
            FadeHitchcockZoomTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
            FadeHitchcockZoomTimeline->SetPlayRate(1.0f);
            FadeHitchcockZoomTimeline->PlayFromStart();

            UE_LOG(LogTemp, Warning, TEXT("Hitchcock Fade In Timeline Started"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FadeInHitchcockCurve is null!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start Hitchcock fade in - missing components"));
    }
}

void UOldManCameraComponent::SetCameraOutHitchcock()
{
    if (FMath::IsNearlyEqual(CurCameraFOV, OriginalCameraFOV, 0.1f) &&
        FMath::IsNearlyEqual(CurCameraDistance, OriginalCameraDistance, 0.1f))
    {
        UE_LOG(LogTemp, Warning, TEXT("SetCameraOutHitchcock: Already at original values"));
        return;
    }

    NotToControlCameraDisState = false;

    if (FadeHitchcockZoomTimeline && CachedOldManCharacter && CachedOldManCharacter->CharacterAttributes)
    {
        FadeHitchcockZoomTimeline->Stop();
        FadeHitchcockZoomTimeline->SetPlaybackPosition(0.0f, false);
        FadeHitchcockZoomTimeline->SetFloatCurve(nullptr, FName("FadeOutHitchcock"));

        auto fadeOutCurve = CachedOldManCharacter->CharacterAttributes->OldManCameraHitchcockData.FadeOutHitchcockCurve;
        if (fadeOutCurve)
        {
            FadeHitchcockZoomTimeline->AddInterpFloat(fadeOutCurve, FadeOutHitchcockTimeLineFloat, FName("FadeOutHitchcock"));
            FadeHitchcockZoomTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
            FadeHitchcockZoomTimeline->SetPlayRate(1.0f);
            FadeHitchcockZoomTimeline->PlayFromStart();

            UE_LOG(LogTemp, Warning, TEXT("Hitchcock Fade Out Timeline Started"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FadeOutHitchcockCurve is null!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to start Hitchcock fade out - missing components"));
    }
}

void UOldManCameraComponent::OnHitchcockTimelineFinishedCallback()
{
    UE_LOG(LogTemp, Warning, TEXT("Hitchcock Timeline Finished"));
}

void UOldManCameraComponent::GetActorsInCone(
    FOldManDetectionData DetectionData,
    const FName ValidTag,
    TArray<AActor*>& OutActors,
    TArray<float>& OutDistances,
    TArray<float>& OutAngles
)
{
    if (!FollowCamera) return;

    FVector Origin = FollowCamera->GetComponentLocation();
    FVector Direction = FollowCamera->GetForwardVector();

    GetActorsInConeInternal(Origin, Direction, DetectionData, ValidTag, OutActors, OutDistances, OutAngles);
}

void UOldManCameraComponent::GetActorsInConeByMousePosition(
    const FVector2D& MouseScreenPosition,
    FOldManDetectionData DetectionData,
    const FName ValidTag,
    TArray<AActor*>& OutActors,
    TArray<float>& OutDistances,
    TArray<float>& OutAngles
)
{
    UWorld* World = GetWorld();
    if (!World || !FollowCamera) return;

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController) return;

    FVector WorldLocation, WorldDirection;
    if (UGameplayStatics::DeprojectScreenToWorld(PlayerController, MouseScreenPosition, WorldLocation, WorldDirection))
    {
        GetActorsInConeInternal(WorldLocation, WorldDirection, DetectionData, ValidTag, OutActors, OutDistances, OutAngles);
    }
}

void UOldManCameraComponent::UpdateCameraData(USpringArmComponent* InCameraBoom, FOldManCameraData CameraData)
{
    MyCameraData = CameraData;

    CameraBoom = InCameraBoom;
    OriginalCameraDistance = MyCameraData.CameraDistance;
    OriginalCameraFOV = MyCameraData.CameraFOV;
    CurCameraDistance = MyCameraData.CameraDistance;
    CurCameraFOV = MyCameraData.CameraFOV;
    InputSmoothingInterpSpeed = MyCameraData.InputSmoothingInterpSpeed;
    GravityRotationInterpSpeed = MyCameraData.GravityRotationInterpSpeed;
    CameraFrameRate = MyCameraData.CameraFrameRate;

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = MyCameraData.CameraDistance;
        CameraBoom->SocketOffset = MyCameraData.CameraOffset;
        CameraBoom->CameraLagSpeed = MyCameraData.CameraLagSpeed;
        CameraBoom->CameraRotationLagSpeed = MyCameraData.CameraRotationLagSpeed;
    }
}

void UOldManCameraComponent::GetActorsInConeInternal(
    const FVector& Origin,
    const FVector& Direction,
    FOldManDetectionData DetectionData,
    const FName ValidTag,
    TArray<AActor*>& OutActors,
    TArray<float>& OutDistances,
    TArray<float>& OutAngles
)
{
给    OutActors.Empty();
    OutDistances.Empty();
    OutAngles.Empty();

    UWorld* World = GetWorld();
    if (!World) return;

    bool bShowDebug = DetectionData.DebugMode;
    float ConeLength = DetectionData.ConeLength;
    float ConeAngle = DetectionData.ConeAngle;

    FVector NormalizedDirection = Direction.GetSafeNormal();
    float HalfConeAngle = ConeAngle * 0.5f;
    float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(HalfConeAngle));

    if (bShowDebug)
    {
        DrawConeVisualization(World, Origin, Direction, ConeLength, ConeAngle, DetectionData.DebugColor, DetectionData.DebugDuration);
    }

    if (DetectionData.DetectionChannels.Num() == 0)
    {
        DetectionData.DetectionChannels.Add(ECC_Pawn);
    }

    for (ECollisionChannel Channel : DetectionData.DetectionChannels)
    {
        TArray<FOverlapResult> OverlapResults;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetOwner());

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

            if (OutActors.Contains(Actor)) continue;

            if (ValidTag != NAME_None && Actor->Tags.Find(ValidTag) < 0) continue;

            FVector ActorLocation = Actor->GetActorLocation();
            FVector ToActor = ActorLocation - Origin;
            float Distance = ToActor.Size();

            if (Distance > ConeLength || Distance < KINDA_SMALL_NUMBER) continue;

            FVector ToActorNormalized = ToActor.GetSafeNormal();
            float DotProduct = FVector::DotProduct(NormalizedDirection, ToActorNormalized);

            if (DotProduct >= CosHalfAngle)
            {
                float Angle = FMath::Acos(DotProduct) * (180.0f / PI);

                FHitResult HitResult;
                FCollisionQueryParams LineParams;
                LineParams.AddIgnoredActor(GetOwner());
                LineParams.AddIgnoredActor(Actor);

                if (World->LineTraceSingleByChannel(HitResult, Origin, ActorLocation, Channel, LineParams))
                {
                    continue;
                }

                OutActors.Add(Actor);
                OutDistances.Add(Distance);
                OutAngles.Add(Angle);
            }
        }
    }
}

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

    float HalfConeAngleRad = FMath::DegreesToRadians(ConeAngle * 0.5f);
    float ConeRadius = ConeLength * FMath::Tan(HalfConeAngleRad);
    FRotator CameraRotation = Direction.Rotation();

    DrawDebugCone(
        World,
        Origin,
        Direction,
        ConeLength,
        HalfConeAngleRad,
        HalfConeAngleRad,
        16,
        Color,
        false,
        Duration,
        0,
        2.0f
    );

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

    if (GEngine)
    {
        FString DebugText = FString::Printf(TEXT("Detection Cone: %.1fm, %.1f°"), ConeLength, ConeAngle);
        GEngine->AddOnScreenDebugMessage(-1, Duration, Color, DebugText);
    }
}