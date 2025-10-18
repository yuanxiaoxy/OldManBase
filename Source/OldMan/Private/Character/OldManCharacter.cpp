#include "Character/OldManCharacter.h"
#include "Character/OldManPersonPlayerController.h"
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
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->CameraRotationLagSpeed = 10.0f;
    CameraBoom->bDoCollisionTest = true;

    // ��������������
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // ��������������
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));

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
    InitializeParam();
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
}

#pragma region Control Param
void AOldManCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;

    // ��������״̬�л�������״̬����ʾ��أ�
    if (PrevMovementMode == MOVE_Falling && NewMovementMode == MOVE_Walking)
    {
        LastLandingTime = GetWorld()->GetTimeSeconds();
        bWasFalling = false;

        UE_LOG(LogTemp, Log, TEXT("Character landed successfully"));
    }
    // ��⿪ʼ����
    else if (NewMovementMode == MOVE_Falling)
    {
        bWasFalling = true;
        UE_LOG(LogTemp, Log, TEXT("Character started falling"));
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

void AOldManCharacter::SetMovementInput(FVector inputDir)
{
    MovementInputVector = inputDir;
}

void AOldManCharacter::SetJumpInput(bool bJumping)
{
    bHasJumpInput = bJumping;
}

void AOldManCharacter::SetAttackInput(bool bAttacking)
{
    bHasAttackInput = bAttacking;
}

void AOldManCharacter::SetPullItemState(bool bPulling)
{
    bHasPullItem = bPulling;
}


void AOldManCharacter::SetRunning(bool bRunning)
{
    bIsRunning = bRunning;
}

FVector AOldManCharacter::GetMovementDirectionFromCamera() const
{
    if (CameraComponent && !MovementInputVector.IsNearlyZero())
    {
        FRotator CameraRotation = CameraComponent->GetCameraRotation();
        CameraRotation.Pitch = 0.0f;
        CameraRotation.Roll = 0.0f;
        return CameraRotation.RotateVector(MovementInputVector);
    }
    return MovementInputVector;
}

void AOldManCharacter::UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection)
{
    if (DesiredDirection.IsNearlyZero())
        return;

    FRotator CurrentRotation = GetActorRotation();
    FRotator TargetRotation = DesiredDirection.Rotation();

    // ������ת���죬����С�ǶȵĶ���
    float YawDifference = FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw);
    if (YawDifference > 1.0f)
    {
        FRotator NewRotation = FMath::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            CharacterAttributes ? CharacterAttributes->RotationBlendInterpSpeed : 8.0f
        );
        SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
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
    // ʹ�ø��ɿ��ļ�ⷽ��
    if (!GetCharacterMovement())
        return false;

    // ����ƶ����˵�����䣬����û�и���أ�����Ϊ������
    bool bMovementFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // ��ֹ�����ʱ��Ȼ����true
    if (!bMovementFalling && (CurrentTime - LastLandingTime < 0.1f))
    {
        return false; // ����أ���������
    }

    return bMovementFalling;
}

bool AOldManCharacter::CanDoubleJump() const
{
    //�ж��Ƿ�����
    return !hasIntoDoubleJump;
}

bool AOldManCharacter::CanAttack() const
{
    if (!IsAlive()) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= (CharacterAttributes ? CharacterAttributes->AttackCooldown : 1.0f);
}

bool AOldManCharacter::HasMovementInput() const
{
    return !MovementInputVector.IsNearlyZero();
}

bool AOldManCharacter::IsActuallyGrounded() const
{
    if (!GetCharacterMovement())
        return false;

    // ʹ�ö��������ж��Ƿ�����ڵ���
    bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
    bool bIsFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // ����ƶ����˵�ڵ��棬�������������¼�������Ϊ����ڵ���
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

void AOldManCharacter::PerformAttackDetection()
{
    if (!CharacterAttributes) return;

    // ��������߼�
    FVector StartLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector();
    FVector EndLocation = StartLocation + ForwardVector * CharacterAttributes->AttackRange;

    // ���μ��
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(50.0f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && HitActor != this)
            {
                // Ӧ���˺��򴥷��¼�
                OnAttackHit(HitActor);
                UE_LOG(LogTemp, Log, TEXT("Hit actor: %s"), *HitActor->GetName());
            }
        }
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
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

void AOldManCharacter::InitializeParam()
{
    // ��ʼ������
    bIsRunning = false;
    hasIntoDoubleJump = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;
    bHasJumpInput = false;
    bHasAttackInput = false;

    // ��ؼ��Ľ�
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
        CameraComponent->InitializeCameraComponents(CameraBoom, FollowCamera);
        CameraComponent->SetCameraTarget(this);
    }
}
#pragma endregion

#pragma region Item Fun
//ʹ��������Tag�жϵ�ǰ�Ƿ��п��϶���Ʒ�ܿ���
void AOldManCharacter::StartRightMousePull()
{
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

    // ���Ի���
    DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Cyan, false, 5.0f, 0, 2.0f);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        AOldManPullItemBase* HitActor = Cast<AOldManPullItemBase>(HitResult.GetActor());
        if (HitActor)
        {
            SetPullItemState(true);
            curOldManPullItem = HitActor;

            // �������е�
            DrawDebugSphere(GetWorld(), HitResult.Location, 15.0f, 12, FColor::Magenta, false, 5.0f, 0, 3.0f);
        }
    }
}

void AOldManCharacter::StopRightMousePull()
{
    SetPullItemState(false);
    curOldManPullItem = nullptr;
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
#pragma endregion
