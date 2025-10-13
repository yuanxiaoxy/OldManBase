#include "Character/OldManCharacter.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"

AOldManCharacter::AOldManCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 创建弹簧臂组件
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraDistance;
    CameraBoom->SocketOffset = CameraOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraLagSpeed = CameraLagSpeed;
    CameraBoom->CameraRotationLagSpeed = CameraRotationLagSpeed;
    CameraBoom->bDoCollisionTest = true;

    // 创建跟随相机组件
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // 创建相机控制组件
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));

    // 初始化变量
    bIsRunning = false;
    bCanDoubleJump = false;
    bJustLanded = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;

    // 新增变量初始化
    TargetCharacterRotation = FRotator::ZeroRotator;

    // 确保角色不自动朝向移动方向，由我们手动控制
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }
}

void AOldManCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeCameraComponent();
    InitializeStateMachine();
}

void AOldManCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新状态机
    if (StateMachine && StateMachine->IsRunning())
    {
        StateMachine->Update(DeltaTime);
    }

    // 重置落地标志
    if (bJustLanded && !IsFalling())
    {
        bJustLanded = false;
    }
}

void AOldManCharacter::Jump()
{
    if (IsAlive())
    {
        Super::Jump();
    }
}

void AOldManCharacter::StopJumping()
{
    Super::StopJumping();
}

void AOldManCharacter::Move(FVector inputDir)
{
    MovementInputVector = inputDir;

    if (!inputDir.IsNearlyZero() && Controller && Controller->IsValidLowLevel())
    {
        // 基于相机方向计算移动方向
        if (CameraComponent)
        {
            FRotator CameraRotation = CameraComponent->GetCameraRotation();
            CameraRotation.Pitch = 0.0f;
            CameraRotation.Roll = 0.0f;

            const FVector Direction = CameraRotation.RotateVector(inputDir);
            AddMovementInput(Direction);

            // 只有在有移动输入时才设置目标旋转并启用旋转更新
            if (!Direction.IsZero())
            {
                TargetCharacterRotation = Direction.Rotation();

                // 立即调用一次旋转更新，确保及时响应
                UpdateCharacterRotation(GetWorld()->GetDeltaSeconds());
            }
        }
    }
    else if (inputDir.IsNearlyZero())
    {
        MovementInputVector = FVector::Zero();
    }
}

void AOldManCharacter::UpdateCharacterRotation(float DeltaTime)
{
    FRotator CurrentRotation = GetActorRotation();

    // 计算旋转差异，避免小角度的抖动
    float YawDifference = FMath::Abs(CurrentRotation.Yaw - TargetCharacterRotation.Yaw);
    if (YawDifference > 1.0f) // 1度以上的差异才需要旋转
    {
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            TargetCharacterRotation,
            DeltaTime,
            CharacterRotationInterpSpeed
        );

        SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
    }
    else
    {
        // 如果差异很小，直接设置
        SetActorRotation(FRotator(0.0f, TargetCharacterRotation.Yaw, 0.0f));
    }
}

void AOldManCharacter::StartRunning()
{
    bIsRunning = true;
}

void AOldManCharacter::StopRunning()
{
    bIsRunning = false;
}

void AOldManCharacter::Attack()
{
    if (CanAttack())
    {
        LastAttackTime = GetWorld()->GetTimeSeconds();
    }
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

void AOldManCharacter::SetThirdPersonMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetThirdPersonMode();
    }
}

void AOldManCharacter::SetFirstPersonMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetFirstPersonMode();
    }
}

void AOldManCharacter::SetFreeLookMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetFreeLookMode();
    }
}

void AOldManCharacter::ShakeCamera(float Intensity, float Duration)
{
    if (CameraComponent)
    {
        CameraComponent->ShakeCamera(Intensity, Duration);
    }
}

bool AOldManCharacter::IsMoving() const
{
    return GetVelocity().SizeSquared() > 0.1f;
}

bool AOldManCharacter::IsFalling() const
{
    return GetCharacterMovement() && GetCharacterMovement()->IsFalling();
}

bool AOldManCharacter::CanDoubleJump() const
{
    return bCanDoubleJump && IsFalling();
}

bool AOldManCharacter::CanAttack() const
{
    if (!IsAlive()) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= CharacterAttributes->AttackCooldown;
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
        CameraComponent->InitializeCameraComponents(CameraBoom, FollowCamera);
        CameraComponent->SetCameraTarget(this);
    }
}