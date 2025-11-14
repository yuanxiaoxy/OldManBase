// OldManCameraAnimationComponent.cpp
#include "CameraAnimation/OldManCameraAnimationComponent.h"
#include "Character/OldManCameraComponent.h"
#include "Character/OldManCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UOldManCameraAnimationComponent::UOldManCameraAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UOldManCameraAnimationComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UOldManCameraAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsAnimationPlaying)
    {
        UpdateCameraAnimation(DeltaTime);
    }
}

void UOldManCameraAnimationComponent::InitializeCameraAnimation(UOldManCameraComponent* InCameraComponent, AOldManCharacter* InCharacter)
{
    CameraComponent = InCameraComponent;
    Character = InCharacter;
}

void UOldManCameraAnimationComponent::StartCameraAnimation(const FOldManCameraAnimationData& AnimationData)
{
    if (!CameraComponent || !Character)
    {
        UE_LOG(LogTemp, Error, TEXT("CameraAnimationComponent not properly initialized!"));
        return;
    }

    CurrentAnimationData = AnimationData;
    bIsAnimationPlaying = true;
    bIsBlendingIn = true;
    bIsBlendingOut = false;
    AnimationBlendAlpha = 0.0f;

    // 保存原始相机数据
    OriginalCameraLocation = CameraComponent->GetCameraLocation();
    OriginalCameraRotation = CameraComponent->GetCameraRotation();
    OriginalCameraDistance = CameraComponent->CurCameraDistance;
    OriginalCameraFOV = CameraComponent->CurCameraFOV;

    // 重置动画状态
    CurrentAnimationData.CurrentPathIndex = 0;
    CurrentAnimationData.PathProgress = 0.0f;
    CurrentAnimationData.CurrentOrbitAngle = 0.0f;

    // 触发委托
    OnCameraAnimationStarted.Broadcast(CurrentAnimationData);

    UE_LOG(LogTemp, Log, TEXT("Started camera animation: %s"),
        *UEnum::GetValueAsString(CurrentAnimationData.AnimationType));
}

void UOldManCameraAnimationComponent::StopCameraAnimation()
{
    if (!bIsAnimationPlaying) return;

    bIsBlendingOut = true;
    bIsBlendingIn = false;

    UE_LOG(LogTemp, Log, TEXT("Stopping camera animation"));
}

void UOldManCameraAnimationComponent::UpdateCameraAnimation(float DeltaTime)
{
    // 更新混合
    UpdateBlend(DeltaTime);

    if (AnimationBlendAlpha <= 0.0f && bIsBlendingOut)
    {
        // 动画结束
        bIsAnimationPlaying = false;
        bIsBlendingOut = false;
        OnCameraAnimationFinished.Broadcast();
        return;
    }

    // 计算动画相机变换
    FVector AnimatedLocation = CalculateAnimatedCameraLocation();
    FRotator AnimatedRotation = CalculateAnimatedCameraRotation();

    // 应用变换
    ApplyCameraTransform(AnimatedLocation, AnimatedRotation, AnimationBlendAlpha);
}

void UOldManCameraAnimationComponent::UpdateBlend(float DeltaTime)
{
    if (bIsBlendingIn)
    {
        float BlendTime = CurrentAnimationData.BlendInTime > 0.0f ? CurrentAnimationData.BlendInTime : 0.1f;
        AnimationBlendAlpha += DeltaTime / BlendTime;

        if (AnimationBlendAlpha >= 1.0f)
        {
            AnimationBlendAlpha = 1.0f;
            bIsBlendingIn = false;
        }
    }
    else if (bIsBlendingOut)
    {
        float BlendTime = CurrentAnimationData.BlendOutTime > 0.0f ? CurrentAnimationData.BlendOutTime : 0.1f;
        AnimationBlendAlpha -= DeltaTime / BlendTime;

        if (AnimationBlendAlpha <= 0.0f)
        {
            AnimationBlendAlpha = 0.0f;
        }
    }

    // 应用混合曲线
    if (CurrentAnimationData.BlendCurve)
    {
        AnimationBlendAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(AnimationBlendAlpha);
    }
}

FVector UOldManCameraAnimationComponent::CalculateAnimatedCameraLocation()
{
    FVector TargetLocation = FVector::ZeroVector;

    switch (CurrentAnimationData.AnimationType)
    {
    case ECameraAnimationType::LookAtObject:
        if (CurrentAnimationData.TargetObject)
        {
            TargetLocation = CurrentAnimationData.TargetObject->GetActorLocation() + CurrentAnimationData.TargetOffset;

            switch (CurrentAnimationData.MovementType)
            {
            case ECameraMovementType::FollowObject:
                // 直接使用目标位置
                break;

            case ECameraMovementType::FixedPosition:
                // 保持初始位置
                TargetLocation = OriginalCameraLocation;
                break;

            case ECameraMovementType::OrbitObject:
                UpdateOrbitMovement(UGameplayStatics::GetWorldDeltaSeconds(this));
                FVector OrbitOffset = FVector(
                    FMath::Cos(FMath::DegreesToRadians(CurrentAnimationData.CurrentOrbitAngle)) * CurrentAnimationData.OrbitRadius,
                    FMath::Sin(FMath::DegreesToRadians(CurrentAnimationData.CurrentOrbitAngle)) * CurrentAnimationData.OrbitRadius,
                    0.0f
                );
                TargetLocation = CurrentAnimationData.TargetObject->GetActorLocation() + OrbitOffset + CurrentAnimationData.TargetOffset;
                break;
            }
        }
        break;

    case ECameraAnimationType::LookAtDirection:
        switch (CurrentAnimationData.MovementType)
        {
        case ECameraMovementType::FixedPosition:
            TargetLocation = OriginalCameraLocation;
            break;

        case ECameraMovementType::FollowPath:
            UpdatePathMovement(UGameplayStatics::GetWorldDeltaSeconds(this));
            if (CurrentAnimationData.PathPoints.Num() > 0)
            {
                // 计算路径插值
                int32 NextIndex = (CurrentAnimationData.CurrentPathIndex + 1) % CurrentAnimationData.PathPoints.Num();
                float Alpha = FMath::Clamp(CurrentAnimationData.PathProgress - CurrentAnimationData.CurrentPathIndex, 0.0f, 1.0f);

                if (CurrentAnimationData.PathCurve)
                {
                    Alpha = CurrentAnimationData.PathCurve->GetFloatValue(Alpha);
                }

                TargetLocation = FMath::Lerp(
                    CurrentAnimationData.PathPoints[CurrentAnimationData.CurrentPathIndex],
                    CurrentAnimationData.PathPoints[NextIndex],
                    Alpha
                );
            }
            break;
        }
        break;
    }

    return TargetLocation;
}

FRotator UOldManCameraAnimationComponent::CalculateAnimatedCameraRotation()
{
    FRotator TargetRotation = FRotator::ZeroRotator;

    switch (CurrentAnimationData.AnimationType)
    {
    case ECameraAnimationType::LookAtObject:
        if (CurrentAnimationData.TargetObject)
        {
            FVector CameraLocation = CalculateAnimatedCameraLocation();
            FVector TargetLocation = CurrentAnimationData.TargetObject->GetActorLocation() + CurrentAnimationData.TargetOffset;
            FVector LookDirection = (TargetLocation - CameraLocation).GetSafeNormal();
            TargetRotation = LookDirection.Rotation();
        }
        break;

    case ECameraAnimationType::LookAtDirection:
        TargetRotation = CurrentAnimationData.TargetRotation;
        break;
    }

    return TargetRotation;
}

void UOldManCameraAnimationComponent::UpdateOrbitMovement(float DeltaTime)
{
    CurrentAnimationData.CurrentOrbitAngle += CurrentAnimationData.OrbitSpeed * DeltaTime;
    if (CurrentAnimationData.CurrentOrbitAngle >= 360.0f)
    {
        CurrentAnimationData.CurrentOrbitAngle -= 360.0f;
    }
}

void UOldManCameraAnimationComponent::UpdatePathMovement(float DeltaTime)
{
    if (CurrentAnimationData.PathPoints.Num() <= 1) return;

    CurrentAnimationData.PathProgress += DeltaTime * CurrentAnimationData.PathSpeed;

    float PathLength = CurrentAnimationData.PathPoints.Num() - 1;
    if (CurrentAnimationData.PathProgress >= PathLength)
    {
        if (CurrentAnimationData.bLoopPath)
        {
            CurrentAnimationData.PathProgress = 0.0f;
        }
        else
        {
            CurrentAnimationData.PathProgress = PathLength;
            // 可以选择在这里停止动画
            // StopCameraAnimation();
        }
    }

    // 确保索引在有效范围内
    CurrentAnimationData.CurrentPathIndex = FMath::Clamp(
        FMath::FloorToInt(CurrentAnimationData.PathProgress),
        0,
        CurrentAnimationData.PathPoints.Num() - 2
    );
}

void UOldManCameraAnimationComponent::ApplyCameraTransform(const FVector& Location, const FRotator& Rotation, float Alpha)
{
    if (!CameraComponent) return;

    // 混合原始位置和动画位置
    FVector FinalLocation = FMath::Lerp(OriginalCameraLocation, Location, Alpha);
    FRotator FinalRotation = FMath::Lerp(OriginalCameraRotation, Rotation, Alpha);

    // 应用相机距离和FOV
    float FinalDistance = FMath::Lerp(OriginalCameraDistance, CurrentAnimationData.CameraDistance, Alpha);
    float FinalFOV = FMath::Lerp(OriginalCameraFOV, CurrentAnimationData.CameraFOV, Alpha);

    // 应用相机变换
    CameraComponent->SetCameraDistance(FinalDistance);
    CameraComponent->SetCameraOffset(CurrentAnimationData.CameraOffset);

    // 设置相机旋转
    if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
    {
        PlayerController->SetControlRotation(FinalRotation);
    }
}