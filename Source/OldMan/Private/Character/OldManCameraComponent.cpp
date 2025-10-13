#include "Character/OldManCameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

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
        DesiredCameraRotation.Pitch = FMath::Clamp(DesiredCameraRotation.Pitch, CameraPitchMin, CameraPitchMax);
    }

    // 平滑插值相机旋转
    if (bUseCameraSmoothing)
    {
        CurrentCameraRotation = FMath::RInterpTo(
            CurrentCameraRotation,
            DesiredCameraRotation,
            DeltaTime,
            CameraRotationInterpSpeed
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

void UOldManCameraComponent::InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera)
{
    CameraBoom = InCameraBoom;
    FollowCamera = InFollowCamera;

    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraDistance;
        CameraBoom->SocketOffset = CameraOffset;
        CameraBoom->CameraLagSpeed = CameraLagSpeed;
        CameraBoom->CameraRotationLagSpeed = CameraRotationLagSpeed;
    }
}

void UOldManCameraComponent::SetCameraTarget(AActor* targetActor)
{
    this->TargetActor = targetActor;
}

void UOldManCameraComponent::SetCameraOffset(const FVector& Offset)
{
    CameraOffset = Offset;
    if (CameraBoom)
    {
        CameraBoom->SocketOffset = CameraOffset;
    }
}

void UOldManCameraComponent::SetCameraDistance(float Distance)
{
    CameraDistance = Distance;
    if (CameraBoom)
    {
        CameraBoom->TargetArmLength = CameraDistance;
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
        CameraBoom->TargetArmLength = CameraDistance;
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
        CameraBoom->TargetArmLength = CameraDistance;
        CameraBoom->bUsePawnControlRotation = true;
        CameraBoom->bEnableCameraLag = true;
        CameraBoom->bEnableCameraRotationLag = true;
        FollowCamera->bUsePawnControlRotation = false;
    }
}