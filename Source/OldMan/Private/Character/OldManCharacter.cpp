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

    // �������ɱ����
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

    // ��������������
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ��������������
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));

    // ��ʼ������
    bIsRunning = false;
    bCanDoubleJump = false;
    bJustLanded = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;

    // ����������ʼ��
    TargetCharacterRotation = FRotator::ZeroRotator;

    // ȷ����ɫ���Զ������ƶ������������ֶ�����
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

    // ����״̬��
    if (StateMachine && StateMachine->IsRunning())
    {
        StateMachine->Update(DeltaTime);
    }

    // ������ر�־
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
        // ���������������ƶ�����
        if (CameraComponent)
        {
            FRotator CameraRotation = CameraComponent->GetCameraRotation();
            CameraRotation.Pitch = 0.0f;
            CameraRotation.Roll = 0.0f;

            const FVector Direction = CameraRotation.RotateVector(inputDir);
            AddMovementInput(Direction);

            // ֻ�������ƶ�����ʱ������Ŀ����ת��������ת����
            if (!Direction.IsZero())
            {
                TargetCharacterRotation = Direction.Rotation();

                // ��������һ����ת���£�ȷ����ʱ��Ӧ
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

    // ������ת���죬����С�ǶȵĶ���
    float YawDifference = FMath::Abs(CurrentRotation.Yaw - TargetCharacterRotation.Yaw);
    if (YawDifference > 1.0f) // 1�����ϵĲ������Ҫ��ת
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
        // ��������С��ֱ������
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

// ========== ������ƺ��� ==========

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