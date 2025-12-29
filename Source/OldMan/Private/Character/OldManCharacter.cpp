#include "Character/OldManCharacter.h"
#include "Character/OldManPersonPlayerController.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManIdleState.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GlobalTagName.h"
#include "GlobalEventName.h"
#include "ItemBase/OldManCableBase.h"
#include "Character/OldManAnimInstance.h"

AOldManCharacter::AOldManCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UOldManMovementComponent>(AOldManCharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    //创建移动组件
    OldManMovementComponent = Cast<UOldManMovementComponent>(Super::GetMovementComponent());

    // 创建碰撞组件
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("Player"));

    // 创建绳索检测Box碰撞器
    CableDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CableDetectionBox"));
    CableDetectionBox->SetupAttachment(RootComponent);
    CableDetectionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CableDetectionBox->SetGenerateOverlapEvents(true);

    // 设置Box大小和位置 - 放在角色脚下
    CableDetectionBox->SetBoxExtent(FVector(80.0f, 80.0f, 30.0f));
    CableDetectionBox->SetRelativeLocation(FVector(0, 0, -80.0f)); // 调整到脚下位置

    bulletFirePos = CreateDefaultSubobject<USceneComponent>(TEXT("bulletFirePosition"));
    bulletFirePos->SetupAttachment(GetMesh());

    // 创建弹簧臂组件
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraDistance;
    CameraBoom->SocketOffset = CameraOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->CameraRotationLagSpeed = 10.0f;
    CameraBoom->bDoCollisionTest = true;

    // 创建跟随相机组件
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = true;

    // 创建相机控制组件
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));
    CameraAnimationComponent = CreateDefaultSubobject<UOldManCameraAnimationComponent>(TEXT("CameraAnimationComponent"));

    // 确保角色不自动朝向移动方向，由我们手动控制
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }

    Tags.Add(UGlobalTagName::Tag_Player);
}

void AOldManCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeParam();
    InitializeCameraComponent();
    InitializeAnimationCameraComponent();
    InitializeStateMachine();
    InitializeEvent();

    if (GetMesh()->GetAnimInstance())
    {
        AnimBlueprintClass = Cast<UOldManAnimInstance>(GetMesh()->GetAnimInstance());
    }

}

void AOldManCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新状态机
    if (StateMachine && StateMachine->IsRunning())
    { 
        StateMachine->Update(DeltaTime);
    }
}

#pragma region Control Param
void AOldManCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;

    // 检测从下落状态切换到行走状态（表示落地）
    if (PrevMovementMode == MOVE_Falling && NewMovementMode == MOVE_Walking)
    {
        LastLandingTime = GetWorld()->GetTimeSeconds();
        bWasFalling = false;

        UE_LOG(LogTemp, Log, TEXT("Character landed successfully"));
    }
    // 检测开始下落
    else if (NewMovementMode == MOVE_Falling)
    {
        bWasFalling = true;
        UE_LOG(LogTemp, Log, TEXT("Character started falling"));
    }
    if (OldManController)
    {
        OldManController->SetShowMouseCursor(true);
        FInputModeGameOnly InputMode; // 创建“仅游戏”输入模式
        OldManController->SetInputMode(InputMode);
    }
    else
    {
		UE_LOG(LogTemp, Warning, TEXT("OldManController is null in OnMovementModeChanged"));
    }
}

bool AOldManCharacter::CanJumpInternal_Implementation() const
{
    return true;
}

AOldManPersonPlayerController* AOldManCharacter::GetOldManController()
{
    if (!OldManController)
    {
        OldManController = Cast<AOldManPersonPlayerController>(GetController());
    }
    return OldManController;
}

void AOldManCharacter::SetPlayerCurMoveState(EPlayerBaseMoveState NewMoveState)
{
    if (CurPlayerMoveState != NewMoveState)
    {
        CurPlayerMoveState = NewMoveState;
    }
}

void AOldManCharacter::SetPlayerCurActionState(EPlayerActionState NewActionState)
{
    if (CurPlayerActionState != NewActionState)
    {
        CurPlayerActionState = NewActionState;
    }
}

void AOldManCharacter::SetMovementInput(FVector inputDir)
{
    MovementInputVector = inputDir;
}

void AOldManCharacter::SetJumpInput(bool bJumping)
{
    bHasJumpInput = bJumping;
}

void AOldManCharacter::SetRunning(bool bRunning)
{
    bIsRunning = bRunning;
}

void AOldManCharacter::ChangeSlopeState(bool slopeState)
{
    bIsOnSlope = slopeState;
}

void AOldManCharacter::SetUseCustomGravity(bool CustomGravityOnEnable)
{
    if (CustomGravityOnEnable)
    {
        SetCameraInSlopeMode();
        CameraComponent->SetCameraInHitchcock(CharacterAttributes->OldManCameraHitchcockData.HitchcockZoomTargetFOV,
            CharacterAttributes->OldManCameraHitchcockData.HitchcockZoomTargetDistance);
        UMonoManager::GetInstance()->SetInterval<AOldManCharacter>(0.05f, "PerformGravityRaycast", this,  &AOldManCharacter::SetGravityDirection);
    }
    else
    {
        SetCameraThirdPersonMode();
        CameraComponent->SetCameraOutHitchcock();
        UMonoManager::GetInstance()->ClearTimer("PerformGravityRaycast");
    }
}

void AOldManCharacter::SetGravityDirection()
{
    OldManMovementComponent->SetGravityDirection(PerformGravityRaycast());
}

FVector AOldManCharacter::PerformGravityRaycast()
{
    if (!GetCapsuleComponent())
        return GetGravityDirection();

    // 获取胶囊体信息
    float CapsuleRadius, CapsuleHalfHeight;
    GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

    FVector RayStart = GetActorLocation();

    // 使用当前重力方向作为射线方向
    FVector CurrentGravityDir = GetGravityDirection();
    FVector RayDirection = CurrentGravityDir;
    FVector RayEnd = RayStart + RayDirection * CharacterAttributes->DetectRayLength; // 默认300

    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(this);

    FHitResult Hit;
    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        RayStart,
        RayEnd,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(CapsuleRadius * 0.8f),
        CollisionParams
    );

    // 调试绘制
    if (true) // 可以添加调试开关
    {
        FColor DebugColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() - GetGravityDirection() * 100.0f, DebugColor, false, 0.1f, 0, 2.0f);

        if (bHit)
        {
            DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Yellow, false, 0.1f, 0);
            DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * 50.0f, FColor::Blue, false, 0.1f, 0, 2.0f);
        }
    }

    if (bHit)
    {
        return -Hit.ImpactNormal;
    }

    return GetGravityDirection();
}

// 修改 UpdateCharacterRotation 以兼容自定义重力：
void AOldManCharacter::UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection)
{
    if (DesiredDirection.IsNearlyZero())
        return;

    // 正常重力下的旋转逻辑
    FRotator CurrentRotation = GetActorRotation();
    FRotator TargetRotation = DesiredDirection.Rotation();

    // 计算旋转差异，避免小角度的抖动
    float YawDifference = FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw);

    if (YawDifference > 1.0f)
    {
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            CharacterAttributes ? CharacterAttributes->RotationBlendInterpSpeed : 8.0f
        );
        SetActorRotation(NewRotation);
    }
}

void AOldManCharacter::UpdateCharacterRotationByGravity(float DeltaTime)
{
    //// 正常重力下的旋转逻辑
    FRotator CurrentRotation = GetActorRotation();
    FRotator gravityRotation = CurrentRotation;

    // 如果使用自定义重力，让重力系统处理角色朝向
    if (OldManMovementComponent)
    {
        // 在自定义重力下，让角色始终"站立"在当前的表面上
        FVector NewUp = -OldManMovementComponent->GetGravityDirection();

        // 获取当前的前方向
        FVector CurrentForward = GetActorForwardVector();

        // 将前方向投影到新的"地面"平面上
        FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp).GetSafeNormal();

        // 如果投影后长度为0，使用默认前方向
        if (NewForward.IsNearlyZero())
        {
            // 尝试使用世界前方向
            NewForward = FVector::VectorPlaneProject(FVector(1, 0, 0), NewUp).GetSafeNormal();
            if (NewForward.IsNearlyZero())
            {
                // 如果还是零，使用世界右方向
                NewForward = FVector::VectorPlaneProject(FVector(0, 1, 0), NewUp).GetSafeNormal();
            }
        }

        // 计算右向量
        FVector NewRight = FVector::CrossProduct(NewUp, NewForward).GetSafeNormal();

        // 重新计算前向量以确保正交
        NewForward = FVector::CrossProduct(NewRight, NewUp).GetSafeNormal();

        // 构建旋转矩阵
        gravityRotation = FRotationMatrix::MakeFromXZ(NewForward, NewUp).Rotator();
    }

     FRotator NewRotation = FMath::RInterpTo(
         CurrentRotation,
         gravityRotation,
         DeltaTime,
         CharacterAttributes ? CharacterAttributes->RotationBlendInterpSpeed : 8.0f
     );
     SetActorRotation(NewRotation);
}

FVector AOldManCharacter::GetMovementDirectionFromCamera()
{
    if (CameraComponent && HasMovementInput())
    {
        // 使用新的有效相机旋转方法
        FRotator CameraRotation = GetEffectiveCameraRotation();
        CameraRotation.Pitch = 0.0f;
        CameraRotation.Roll = 0.0f;
        return CameraRotation.RotateVector(MovementInputVector);
    }
    return MovementInputVector;
}

//更新是否激活输入
void AOldManCharacter::UpdateInputActive(bool active)
{
    InputActive = active;
}

// ========== 相机控制函数 ==========

void AOldManCharacter::SetCameraDistance(float Distance)
{
    if (CameraComponent)
    {
        CameraComponent->SetCameraDistance(Distance);
    }
}

void AOldManCharacter::SetCameraOffset(const FVector& Offset)
{
    if (CameraComponent)
    {
        CameraComponent->SetCameraOffset(Offset);
    }
}

void AOldManCharacter::SetCameraThirdPersonMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetThirdPersonMode();
    }
}

void AOldManCharacter::SetCameraInSlopeMode()
{
    auto a = CameraComponent;

    if (CameraComponent)
    {
        CameraComponent->SetPersonInSlopeMode();
    }
}

void AOldManCharacter::SetCameraMouseCursorMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetMouseCursorMode();
    }
}

ECameraMode AOldManCharacter::GetCurrentCameraMode() const
{
    if (CameraComponent)
    {
        return CameraComponent->GetCurrentCameraMode();
    }
    return ECameraMode::ThirdPersonMode;
}

void AOldManCharacter::ShakeCamera(float Intensity, float Duration)
{
    if (CameraComponent)
    {
        CameraComponent->ShakeCamera(Intensity, Duration);
    }
}

void AOldManCharacter::StopCameraAnimation(bool bImmediate)
{
    if (CameraAnimationComponent)
    {
        CameraAnimationComponent->StopCameraAnimation(bImmediate);
    }
}

void AOldManCharacter::SetCameraAnimationTarget(AActor* TargetActor)
{
    if (CameraAnimationComponent)
    {
        CameraAnimationComponent->SetAnimationTarget(TargetActor);
    }
}

void AOldManCharacter::PauseCameraAnimation()
{
    if (CameraAnimationComponent)
    {
        CameraAnimationComponent->PauseCameraAnimation();
    }
}

bool AOldManCharacter::IsCameraAnimationPlaying() const
{
    return CameraAnimationComponent ? CameraAnimationComponent->IsCameraAnimationPlaying() : false;
}

FRotator AOldManCharacter::GetAnimationCameraRotation() const
{
    if (CameraAnimationComponent)
    {
        return CameraAnimationComponent->GetAnimationCameraRotation();
    }
    return FRotator::ZeroRotator;
}

FRotator AOldManCharacter::GetEffectiveCameraRotation() const
{
    // 检查是否正在播放相机动画，并且该动画启用了使用动画相机控制移动
    if (CameraAnimationComponent && CameraAnimationComponent->IsCameraAnimationPlaying())
    {
        FOldManCameraAnimationData CurrentData = CameraAnimationComponent->GetCurrentAnimationData();
        if (CurrentData.bUseAnimationCameraForMovement)
        {
            // 使用动画相机的旋转
            return CameraAnimationComponent->GetAnimationCameraRotation();
        }
    }

    // 默认使用常规相机旋转
    if (CameraComponent)
    {
        return CameraComponent->GetCameraRotation();
    }

    return FRotator::ZeroRotator;
}

void AOldManCharacter::PlayCameraAnimation(const FOldManCameraAnimationData& AnimationData, bool bForceRestart)
{
    if (CameraAnimationComponent)
    {
        // 设置运行时跟随目标为自己（如果目标是空的且是跟随模式）
        FOldManCameraAnimationData ModifiedData = AnimationData;
        if (!ModifiedData.TargetObject &&
            (ModifiedData.AnimationType == ECameraAnimationType::MoveToTargetAndLookAtPlayer ||
                ModifiedData.AnimationType == ECameraAnimationType::FollowPlayerAndLookAtTarget))
        {
            ModifiedData.TargetObject = this;
        }
        ModifiedData.RuntimeFollowTarget = this;
        CameraAnimationComponent->StartCameraAnimation(ModifiedData, bForceRestart);
    }
}

// 新增：切换相机动画
void AOldManCharacter::SwitchCameraAnimation(const FOldManCameraAnimationData& AnimationData, float TransitionTime)
{
    if (CameraAnimationComponent)
    {
        // 设置运行时跟随目标为自己（如果目标是空的且是跟随模式）
        FOldManCameraAnimationData ModifiedData = AnimationData;
        if (!ModifiedData.TargetObject &&
            (ModifiedData.AnimationType == ECameraAnimationType::MoveToTargetAndLookAtPlayer ||
                ModifiedData.AnimationType == ECameraAnimationType::FollowPlayerAndLookAtTarget))
        {
            ModifiedData.TargetObject = this;
        }
        ModifiedData.RuntimeFollowTarget = this;
        CameraAnimationComponent->SwitchCameraAnimation(ModifiedData, TransitionTime);
    }
}

void AOldManCharacter::SetCameraFollowParameters(const FVector& PositionOffset, float Distance, bool bWithRotation, bool bLookAtTarget)
{
    if (CameraAnimationComponent)
    {
        // 根据当前动画类型设置不同的参数
        FOldManCameraAnimationData CurrentData = CameraAnimationComponent->GetCurrentAnimationData();

        if (CurrentData.AnimationType == ECameraAnimationType::MoveToTargetAndLookAtPlayer)
        {
            CameraAnimationComponent->SetMoveToParameters(PositionOffset, ECameraOffsetSpace::Local);
        }
        else if (CurrentData.AnimationType == ECameraAnimationType::FollowPlayer)
        {
            CameraAnimationComponent->SetFollowPlayerParameters(PositionOffset, ECameraOffsetSpace::Local, bWithRotation);
        }
        else if (CurrentData.AnimationType == ECameraAnimationType::FollowPlayerAndLookAtTarget)
        {
            CameraAnimationComponent->SetFollowPlayerAndLookAtParameters(
                Distance,
                PositionOffset.Z,
                PositionOffset,
                FRotator::ZeroRotator,
                true // 默认使用延长线偏移
            );
        }
    }
}

void AOldManCharacter::PlayFollowPlayerAnimation(const FVector& PositionOffset, bool bWithRotation, AActor* LookAtTarget, const FVector& LookAtOffset, const FRotator& CameraRotationOffset, bool bUseExtensionLineOffset)
{
    if (CameraAnimationComponent)
    {
        FOldManCameraAnimationData AnimationData;

        // 判断使用哪种模式
        if (LookAtTarget)
        {
            AnimationData.AnimationType = ECameraAnimationType::FollowPlayerAndLookAtTarget;
            AnimationData.TargetObject = LookAtTarget;
            AnimationData.TargetOffset = LookAtOffset;

            // 设置球坐标系参数
            AnimationData.SphereRadius = 500.0f; // 默认半径
            AnimationData.SphereHeight = PositionOffset.Z;
            AnimationData.CameraOffset = PositionOffset;
            AnimationData.CameraRotationOffset = CameraRotationOffset;
            AnimationData.bUseExtensionLineOffset = bUseExtensionLineOffset;
        }
        else
        {
            AnimationData.AnimationType = ECameraAnimationType::FollowPlayer;
            AnimationData.FollowCameraOffset = PositionOffset;
            AnimationData.FollowOffsetSpace = ECameraOffsetSpace::Local;
            AnimationData.bFollowWithRotation = bWithRotation;
        }

        AnimationData.RuntimeFollowTarget = this; // 跟随玩家自己

        // 设置混合参数
        AnimationData.BlendInTime = 0.5f;
        AnimationData.BlendOutTime = 0.5f;

        // 行为设置
        AnimationData.bDisablePlayerInput = false; // 保持玩家输入
        AnimationData.bHidePlayer = false;
        AnimationData.bUseAnimationCameraForMovement = true; // 使用动画相机控制移动方向

        CameraAnimationComponent->StartCameraAnimation(AnimationData);
    }
}

void AOldManCharacter::PlayFollowPlayerWithMouseExposure(
    const FVector& PositionOffset,
    bool bWithRotation,
    bool bExposeMousePosition,
    bool bDisablePlayerInput)
{
    if (CameraAnimationComponent)
    {
        FOldManCameraAnimationData AnimationData;
        AnimationData.AnimationType = ECameraAnimationType::FollowPlayer;
        AnimationData.FollowCameraOffset = PositionOffset;
        AnimationData.FollowOffsetSpace = ECameraOffsetSpace::Local;
        AnimationData.bFollowWithRotation = bWithRotation;
        AnimationData.bExposeMousePosition = bExposeMousePosition;
        AnimationData.RuntimeFollowTarget = this; // 跟随玩家自己

        // 设置混合参数
        AnimationData.BlendInTime = 0.5f;
        AnimationData.BlendOutTime = 0.5f;

        // 行为设置
        AnimationData.bDisablePlayerInput = bDisablePlayerInput;
        AnimationData.bHidePlayer = false;
        AnimationData.bUseAnimationCameraForMovement = true; // 使用动画相机控制移动方向

        CameraAnimationComponent->StartCameraAnimation(AnimationData);
    }
}

void AOldManCharacter::UpdateFollowPlayerTarget(AActor* NewTarget)
{
    if (CameraAnimationComponent && CameraAnimationComponent->IsCameraAnimationPlaying())
    {
        CameraAnimationComponent->SetAnimationTarget(NewTarget);
    }
}



///////////////StateCheck
bool AOldManCharacter::IsMoving() const
{
    return GetVelocity().SizeSquared() > 0.1f;
}

bool AOldManCharacter::IsFalling() const
{
    // 使用更可靠的检测方法
    if (!GetCharacterMovement())
        return false;

    // 如果移动组件说在下落，并且没有刚落地，则认为在下落
    bool bMovementFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 防止刚落地时仍然返回true
    if (!bMovementFalling && (CurrentTime - LastLandingTime < 0.1f))
    {
        return false; // 刚落地，不算下落
    }

    return bMovementFalling;
}

bool AOldManCharacter::CanDoubleJump() const
{
    //判断是否进入过
    return !hasIntoDoubleJump;
}

bool AOldManCharacter::HasCable() const
{
    return bHasCable;
}

bool AOldManCharacter::HasMovementInput()
{
    return !MovementInputVector.IsNearlyZero() && !GetOldManController()->IsMoveInputIgnored();
}

bool AOldManCharacter::IsActuallyGrounded() const
{
    if (!GetCharacterMovement())
        return false;

    // 使用多种条件判断是否真的在地面
    bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
    bool bIsFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 如果移动组件说在地面，并且最近有落地事件，则认为真的在地面
    return bIsOnGround && !bIsFalling && (CurrentTime - LastLandingTime < 0.5f);
}

float AOldManCharacter::GetTimeSinceLastLanding() const
{
    return GetWorld()->GetTimeSeconds() - LastLandingTime;
}

void AOldManCharacter::PrintMovementState() const
{
    if (!GetCharacterMovement()) return;

    FString MovementState;
    switch (GetCharacterMovement()->MovementMode)
    {
    case MOVE_None: MovementState = "None"; break;
    case MOVE_Walking: MovementState = "Walking"; break;
    case MOVE_NavWalking: MovementState = "NavWalking"; break;
    case MOVE_Falling: MovementState = "Falling"; break;
    case MOVE_Swimming: MovementState = "Swimming"; break;
    case MOVE_Flying: MovementState = "Flying"; break;
    case MOVE_Custom: MovementState = "Custom"; break;
    default: MovementState = "Unknown"; break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Movement State: %s, IsFalling: %d, IsActuallyGrounded: %d"),
        *MovementState,
        GetCharacterMovement()->IsFalling(),
        IsActuallyGrounded());
}

void AOldManCharacter::SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass)
{
    if (NewMesh)
    {
        GetMesh()->SetSkeletalMesh(NewMesh);
    }

    if (NewAnimClass)
    {
        GetMesh()->SetAnimInstanceClass(NewAnimClass);
    }
}

void AOldManCharacter::DectedActors()
{
    if (!CanFireBullet())
    {
        return;
    }

    TArray<AActor*> OutActors;
    TArray<float> OutDistances;
    TArray<float> OutAngles;

    // 检查是否正在播放相机动画，并且该动画启用了鼠标位置暴露
    if (CameraAnimationComponent && CameraAnimationComponent->IsCameraAnimationPlaying())
    {
        FOldManCameraAnimationData CurrentData = CameraAnimationComponent->GetCurrentAnimationData();

        // 如果当前动画是FollowPlayer并且启用了鼠标位置暴露
        if (CurrentData.AnimationType == ECameraAnimationType::FollowPlayer && CurrentData.bExposeMousePosition)
        {
            // 使用鼠标位置进行检测
            APlayerController* PlayerController = GetOldManController();
            if (PlayerController)
            {
                float MouseX, MouseY;
                if (PlayerController->GetMousePosition(MouseX, MouseY))
                {
                    FVector2D MousePosition(MouseX, MouseY);
                    CameraComponent->GetActorsInConeByMousePosition(
                        MousePosition,
                        CharacterAttributes->OldManDetectionData,
                        UGlobalTagName::Tag_BeDetcedItem,
                        OutActors,
                        OutDistances,
                        OutAngles
                    );
                }
            }
        }
        else
        {
            // 其他情况使用常规检测
            CameraComponent->GetActorsInCone(
                CharacterAttributes->OldManDetectionData,
                UGlobalTagName::Tag_BeDetcedItem,
                OutActors,
                OutDistances,
                OutAngles
            );
        }
    }
    else
    {
        // 没有动画时，根据当前相机模式选择检测方法
        if (CameraComponent && CameraComponent->GetCurrentCameraMode() == ECameraMode::MouseCursorMode)
        {
            // 鼠标光标模式下，使用鼠标位置进行检测
            APlayerController* PlayerController = GetOldManController();
            if (PlayerController)
            {
                float MouseX, MouseY;
                if (PlayerController->GetMousePosition(MouseX, MouseY))
                {
                    FVector2D MousePosition(MouseX, MouseY);
                    CameraComponent->GetActorsInConeByMousePosition(
                        MousePosition,
                        CharacterAttributes->OldManDetectionData,
                        UGlobalTagName::Tag_BeDetcedItem,
                        OutActors,
                        OutDistances,
                        OutAngles
                    );
                }
            }
        }
        else
        {
            // 常规模式下，使用相机位置和方向进行检测
            CameraComponent->GetActorsInCone(
                CharacterAttributes->OldManDetectionData,
                UGlobalTagName::Tag_BeDetcedItem,
                OutActors,
                OutDistances,
                OutAngles
            );
        }
    }

    if (OutActors.Num() != OutDistances.Num() || OutActors.Num() != OutAngles.Num())
    {
        return;
    }

    AActor* finalActor = nullptr;
    float distance = 10000.0f;
    for (int i = 0; i < OutActors.Num(); i++)
    {
        if (distance > OutDistances[i])
        {
            distance = OutDistances[i];
            finalActor = OutActors[i];
        }
        UE_LOG(LogTemp, Display, TEXT("%s"), *(OutActors[i]->GetFName().ToString()));
    }

    InFireCoolDown = true;
    bCouldPullItem = false;

    if (AnimBlueprintClass && CharacterAttributes->AttackMontage)
    {
        AnimBlueprintClass->Montage_Play(CharacterAttributes->AttackMontage);
    }

    UMonoManager::GetInstance()->SetTimeoutWithArgs<AOldManCharacter, AActor*>(
        CharacterAttributes->OldManDetectionData.AnimReadyTime,
        this,
        &AOldManCharacter::FireBullet,
        finalActor);

    UMonoManager::GetInstance()->SetTimeoutLambda(CharacterAttributes->OldManDetectionData.AnimTime, [this]() {
        bCouldPullItem = true;
        });
}

void AOldManCharacter::InitializeParam()
{
    PlayerCurHealth = CharacterAttributes->PlayerHealth;

    // 初始化变量
    bIsRunning = false;
    hasIntoDoubleJump = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;
    bHasJumpInput = false;
    bHasAttackInput = false;
    bInPullState = false;

    // 落地检测改进
    LastLandingTime = 0.0f;
    bWasFalling = false;
}

void AOldManCharacter::InitializeStateMachine()
{
    UStateMachineManager* StateMachineManager = UStateMachineManager::GetStateMachineManager();
    if (StateMachineManager)
    {
        StateMachine = StateMachineManager->CreateStateMachine(this, true);
        StateMachine->InitializeWithState(UOldManIdleState::StaticClass(), this);
    }
}

void AOldManCharacter::InitializeCameraComponent()
{
    if (CameraComponent && CameraBoom && FollowCamera)
    {
        CameraComponent->InitializeCameraComponents(CameraBoom, FollowCamera, CharacterAttributes->OldManCameraData);
        CameraComponent->SetCameraTarget(this);
    }
}

void AOldManCharacter::InitializeAnimationCameraComponent()
{
    // 初始化相机动画组件
    if (CameraAnimationComponent && CameraComponent)
    {
        CameraAnimationComponent->InitializeCameraAnimation(CameraComponent, this);
    }
}

void AOldManCharacter::InitializeEvent()
{
    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, bool>(UGlobalEventName::Key_Player_OnChangeGrivity, this, &AOldManCharacter::ChangeSlopeState);

    UMyEventManager::GetInstance()->RegisterCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), this, &AOldManCharacter::UpdateInputActive);

    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, FGameEventData>(UGlobalEventName::Key_Player_OnRespawn, this, &AOldManCharacter::OnPlayerRespawn);
}
#pragma endregion

#pragma region Item Fun
void AOldManCharacter::FireBullet(AActor* actor)
{
    // 生成子弹
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    AOldManBulletBase* Bullet = GetWorld()->SpawnActor<AOldManBulletBase>(firstKindBullet,
        bulletFirePos->GetComponentLocation(), bulletFirePos->GetComponentRotation(), SpawnParams);

    if (Bullet)
    {
        UMonoManager::GetInstance()->SetTimeout(CharacterAttributes->OldManDetectionData.CoolDown, this, &AOldManCharacter::CancelFireCoolDown);

        FVector bulletDir = GetActorForwardVector();
        if (actor)
        {
            bulletDir = actor->GetActorLocation() - bulletFirePos->GetComponentLocation();
        }
        Bullet->InitializeBullet(bulletDir.GetSafeNormal(), actor);
    }
}

void AOldManCharacter::CancelFireCoolDown()
{
    InFireCoolDown = false;
}

bool AOldManCharacter::CanFireBullet()
{
    return !InFireCoolDown && !bInPullState;
}


void AOldManCharacter::SetPullItemState(bool bPulling)
{
    bInPullState = bPulling;
}

bool AOldManCharacter::GetIfCouldPullItem()
{
    return bCouldPullItem;
}

//使用射线与Tag判断当前是否有可拖动物品能控制
void AOldManCharacter::StartRightMousePull()
{
    if (!GetIfCouldPullItem())
    {
        return;
    }

    if (!GetOldManController() || !GetOldManController()->PlayerCameraManager) return;

    FVector CameraLocation = GetOldManController()->PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = GetOldManController()->PlayerCameraManager->GetCameraRotation();
    FVector CameraDirection = CameraRotation.Vector();

    UE_LOG(LogTemp, Display, TEXT("Camera Raycast - Location: %s, Direction: %s"),
        *CameraLocation.ToString(), *CameraDirection.ToString());

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOldManController()->GetPawn());

    FVector TraceEnd = CameraLocation + CameraDirection * 10000.0f;

    // 调试绘制
    DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Cyan, false, 5.0f, 0, 2.0f);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        AOldManPullItemBase* HitActor = Cast<AOldManPullItemBase>(HitResult.GetActor());
        if (HitActor)
        {
            SetPullItemState(true);
            HitActor->StartDragging();
            curOldManPullItem = HitActor;

            // 绘制命中点
            DrawDebugSphere(GetWorld(), HitResult.Location, 15.0f, 12, FColor::Magenta, false, 5.0f, 0, 3.0f);
        }
    }
}

void AOldManCharacter::StopRightMousePull()
{
    SetPullItemState(false);
    if (curOldManPullItem)
    {
        curOldManPullItem->StopDragging();
        curOldManPullItem = nullptr;
    }
}

void AOldManCharacter::HandleMouseLook(FVector2D mouseDelta)
{
    // 鼠标输入
    float MouseXInput = mouseDelta.X;
    float MouseYInput = mouseDelta.Y;

    // 处理拖动
    if (curOldManPullItem && bInPullState)
    {
        APlayerCameraManager* CameraManager = GetOldManController()->PlayerCameraManager;
        if (CameraManager && (FMath::Abs(MouseXInput) > MinMovementThreshold || FMath::Abs(MouseYInput) > MinMovementThreshold))
        {
            FVector CameraLocation = CameraManager->GetCameraLocation();
            FRotator CameraRotation = CameraManager->GetCameraRotation();

            // 获取相机的方向向量
            FVector CameraForward = CameraRotation.Vector();
            FVector CameraRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
            FVector CameraUp = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Z);

            // 基于鼠标输入构建移动方向
            FVector ViewMovementDirection = (CameraRight * MouseXInput + CameraUp * MouseYInput).GetSafeNormal();

            // 计算移动强度
            float MovementIntensity = FVector2D(MouseXInput, MouseYInput).Size() * DragSensitivity;

            // 应用移动
            curOldManPullItem->HandleMouseData(ViewMovementDirection, MovementIntensity);

            // 重置鼠标输入
            MouseXInput = 0.0f;
            MouseYInput = 0.0f;
        }
    }
}

void AOldManCharacter::SetCurOldManInterectItem(AOldManInterectItemBase* newItem)
{
    if (newItem)
    {
        curOldManInterectItem = newItem;
    }
}

void AOldManCharacter::ClearCurOldManInterectItem()
{
    curOldManInterectItem = nullptr;
}

void AOldManCharacter::InterectCurOldManInterectItem(FOldManItemInteractData interectData)
{
    if (curOldManInterectItem)
    {
        curOldManInterectItem->Interect(interectData);
    }
}

void AOldManCharacter::SetCurrentCable(AOldManCableBase* newCable)
{
    if (newCable == CurrentCable)
    {
        return;
    }

    if (newCable == nullptr)
    {
        bHasCable = false;
        CurrentCable = nullptr;
    }
    else
    {
        bHasCable = true;
        CurrentCable = newCable;
    }
}

void AOldManCharacter::SetNextCable(AOldManCableBase* newCable, bool left)
{
    NextCable = newCable;
    IsLeftCable = left;
}
#pragma endregion

#pragma region Character Param
void AOldManCharacter::OnPlayerRespawn(FGameEventData EventData)
{

    //临时只复位 加切换状态
    InitializeParam();
    InitializeStateMachine();
}

bool AOldManCharacter::IsAlive()
{
    return PlayerCurHealth > 0;
}

float AOldManCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    PlayerCurHealth -= DamageAmount;
    return DamageAmount;
}
#pragma endregion
