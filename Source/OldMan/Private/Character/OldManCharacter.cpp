#include "Character/OldManCharacter.h"
#include "Character/OldManPersonPlayerController.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManRebornState.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GlobalTagName.h"
#include "GlobalEventName.h"
#include "ItemBase/OldManCableBase.h"
#include "Character/OldManAnimInstance.h"
#include "Kismet/GameplayStatics.h" // Newly added for DeprojectScreenToWorld

AOldManCharacter::AOldManCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UOldManMovementComponent>(AOldManCharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    // Create movement component
    OldManMovementComponent = Cast<UOldManMovementComponent>(Super::GetMovementComponent());

    // Create interaction box
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("Player"));

    // Create cable detection box collider
    CableDetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CableDetectionBox"));
    CableDetectionBox->SetupAttachment(RootComponent);
    CableDetectionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    CableDetectionBox->SetGenerateOverlapEvents(true);

    // Set box size and location - placed at character's feet
    CableDetectionBox->SetBoxExtent(FVector(80.0f, 80.0f, 30.0f));
    CableDetectionBox->SetRelativeLocation(FVector(0, 0, -80.0f)); // Adjust to feet position

    bulletFirePos = CreateDefaultSubobject<USceneComponent>(TEXT("bulletFirePosition"));
    bulletFirePos->SetupAttachment(GetMesh());

    // Create camera boom
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

    // Create follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = true;

    // Create camera control component
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));
    CameraAnimationComponent = CreateDefaultSubobject<UOldManCameraAnimationComponent>(TEXT("CameraAnimationComponent"));

    // Ensure character does not auto-orient to movement; we control manually
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
    InitializeEvent();
    InitializeParam();
    InitializeCameraComponent();
    InitializeAnimationCameraComponent();
    InitializeStateMachine();

    if (GetMesh()->GetAnimInstance())
    {
        AnimBlueprintClass = Cast<UOldManAnimInstance>(GetMesh()->GetAnimInstance());
    }
}

void AOldManCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update state machine
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

    // Detect transition from falling to walking (landing)
    if (PrevMovementMode == MOVE_Falling && NewMovementMode == MOVE_Walking)
    {
        LastLandingTime = GetWorld()->GetTimeSeconds();
        bWasFalling = false;

        UE_LOG(LogTemp, Log, TEXT("Character landed successfully"));
    }
    // Detect start of falling
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
        UMonoManager::GetInstance()->SetInterval<AOldManCharacter>(0.05f, "PerformGravityRaycast", this, &AOldManCharacter::SetGravityDirection);
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

    // Get capsule info
    float CapsuleRadius, CapsuleHalfHeight;
    GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

    FVector RayStart = GetActorLocation();

    // Use current gravity direction as ray direction
    FVector CurrentGravityDir = GetGravityDirection();
    FVector RayDirection = CurrentGravityDir;
    FVector RayEnd = RayStart + RayDirection * CharacterAttributes->DetectRayLength; // default 300

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

    // Debug drawing
    if (true) // Can be toggled with a debug switch
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

// Modify UpdateCharacterRotation to be compatible with custom gravity:
void AOldManCharacter::UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection)
{
    if (DesiredDirection.IsNearlyZero())
        return;

    // Rotation logic under normal gravity
    FRotator CurrentRotation = GetActorRotation();
    FRotator TargetRotation = DesiredDirection.Rotation();

    // Calculate rotation difference, avoid jitter for small angles
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
    // Rotation logic under normal gravity
    FRotator CurrentRotation = GetActorRotation();
    FRotator gravityRotation = CurrentRotation;

    // If using custom gravity, let gravity system handle orientation
    if (OldManMovementComponent)
    {
        // Under custom gravity, make character always "stand" on the current surface
        FVector NewUp = -OldManMovementComponent->GetGravityDirection();

        // Get current forward direction
        FVector CurrentForward = GetActorForwardVector();

        // Project forward onto the new ground plane
        FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp).GetSafeNormal();

        // If projection length is zero, use default forward
        if (NewForward.IsNearlyZero())
        {
            // Try world forward
            NewForward = FVector::VectorPlaneProject(FVector(1, 0, 0), NewUp).GetSafeNormal();
            if (NewForward.IsNearlyZero())
            {
                // If still zero, use world right
                NewForward = FVector::VectorPlaneProject(FVector(0, 1, 0), NewUp).GetSafeNormal();
            }
        }

        // Compute right vector
        FVector NewRight = FVector::CrossProduct(NewUp, NewForward).GetSafeNormal();

        // Recompute forward to ensure orthogonality
        NewForward = FVector::CrossProduct(NewRight, NewUp).GetSafeNormal();

        // Build rotation matrix
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
        // Use the effective camera rotation method
        FRotator CameraRotation = GetEffectiveCameraRotation();
        CameraRotation.Pitch = 0.0f;
        CameraRotation.Roll = 0.0f;
        return CameraRotation.RotateVector(MovementInputVector);
    }
    return MovementInputVector;
}

// Update input active state
void AOldManCharacter::UpdateInputActive(bool active)
{
    InputActive = active;
}

void AOldManCharacter::OnInputDeviceChanged(EHardwareDevicePrimaryType InputDevice)
{
    if (InputDevice == EHardwareDevicePrimaryType::Gamepad)
    {
        CharacterAttributes = GamePadCharacterAttributes;
    }
    else if (InputDevice == EHardwareDevicePrimaryType::KeyboardAndMouse)
    {
        CharacterAttributes = KeyBoardCharacterAttributes;
    }

    CameraComponent->UpdateCameraData(CameraBoom, CharacterAttributes->OldManCameraData);
}

// ========== Camera control functions ==========

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
    // Check if camera animation is playing and if it should be used for movement
    if (CameraAnimationComponent && CameraAnimationComponent->IsCameraAnimationPlaying())
    {
        FOldManCameraAnimationData CurrentData = CameraAnimationComponent->GetCurrentAnimationData();
        if (CurrentData.bUseAnimationCameraForMovement)
        {
            // Use animation camera rotation
            return CameraAnimationComponent->GetAnimationCameraRotation();
        }
    }

    // Default to regular camera rotation
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
        // Set runtime follow target to self if target is empty and it's a follow mode
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

// New: Switch camera animation
void AOldManCharacter::SwitchCameraAnimation(const FOldManCameraAnimationData& AnimationData, float TransitionTime)
{
    if (CameraAnimationComponent)
    {
        // Set runtime follow target to self if target is empty and it's a follow mode
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
        // Set different parameters based on current animation type
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
                true // default use extension line offset
            );
        }
    }
}

void AOldManCharacter::PlayFollowPlayerAnimation(const FVector& PositionOffset, bool bWithRotation, AActor* LookAtTarget, const FVector& LookAtOffset, const FRotator& CameraRotationOffset, bool bUseExtensionLineOffset)
{
    if (CameraAnimationComponent)
    {
        FOldManCameraAnimationData AnimationData;

        // Determine which mode to use
        if (LookAtTarget)
        {
            AnimationData.AnimationType = ECameraAnimationType::FollowPlayerAndLookAtTarget;
            AnimationData.TargetObject = LookAtTarget;
            AnimationData.TargetOffset = LookAtOffset;

            // Set spherical coordinate parameters
            AnimationData.SphereRadius = 500.0f; // default radius
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

        AnimationData.RuntimeFollowTarget = this; // Follow player itself

        // Set blend parameters
        AnimationData.BlendInTime = 0.5f;
        AnimationData.BlendOutTime = 0.5f;

        // Behavior settings
        AnimationData.bDisablePlayerInput = false; // Keep player input
        AnimationData.bHidePlayer = false;
        AnimationData.bUseAnimationCameraForMovement = true; // Use animation camera for movement direction

        CameraAnimationComponent->StartCameraAnimation(AnimationData);
    }
}

void AOldManCharacter::PlayFollowPlayerWithMouseExposure(
    const FVector& PositionOffset,
    bool bWithRotation,
    bool bExposeMousePosition,
    bool bDisablePlayerInput)  // Added this parameter
{
    if (CameraAnimationComponent)
    {
        FOldManCameraAnimationData AnimationData;
        AnimationData.AnimationType = ECameraAnimationType::FollowPlayer;
        AnimationData.FollowCameraOffset = PositionOffset;
        AnimationData.FollowOffsetSpace = ECameraOffsetSpace::Local;
        AnimationData.bFollowWithRotation = bWithRotation;
        AnimationData.bExposeMousePosition = bExposeMousePosition;
        AnimationData.bDisablePlayerInput = bDisablePlayerInput;  // Use passed parameter
        AnimationData.RuntimeFollowTarget = this; // Follow player itself

        // Set blend parameters
        AnimationData.BlendInTime = 0.5f;
        AnimationData.BlendOutTime = 0.5f;

        // Behavior settings
        AnimationData.bHidePlayer = false;
        AnimationData.bUseAnimationCameraForMovement = true; // Use animation camera for movement direction

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
    // Use more reliable detection
    if (!GetCharacterMovement())
        return false;

    // If movement component says falling, and we haven't just landed, consider falling
    bool bMovementFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Prevent returning true right after landing
    if (!bMovementFalling && (CurrentTime - LastLandingTime < 0.1f))
    {
        return false; // Just landed, not falling
    }

    return bMovementFalling;
}

bool AOldManCharacter::CanDoubleJump() const
{
    // Check if already entered
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

    // Use multiple conditions to determine if truly grounded
    bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
    bool bIsFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // If movement component says on ground and we recently landed, consider truly grounded
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

// ========== Modify DectedActors to use unified cursor position ==========
void AOldManCharacter::DectedActors()
{
    if (!CanFireBullet())
    {
        return;
    }

    AOldManPersonPlayerController* PC = GetOldManController();
    if (!PC)
        return;

    TArray<AActor*> OutActors;
    TArray<float> OutDistances;
    TArray<float> OutAngles;

    // Get effective cursor screen position (gamepad virtual cursor or real mouse)
    FVector2D CursorScreenPos = PC->GetEffectiveCursorScreenPosition();

    // Check if camera animation is playing and exposes mouse position
    bool bUseCursorPos = false;
    if (CameraAnimationComponent && CameraAnimationComponent->IsCameraAnimationPlaying())
    {
        FOldManCameraAnimationData CurrentData = CameraAnimationComponent->GetCurrentAnimationData();
        if (CurrentData.AnimationType == ECameraAnimationType::FollowPlayer && CurrentData.bExposeMousePosition)
        {
            bUseCursorPos = true;
        }
    }
    else if (CameraComponent && CameraComponent->GetCurrentCameraMode() == ECameraMode::MouseCursorMode)
    {
        bUseCursorPos = true;
    }

    if (bUseCursorPos || PC->ShouldUseCursorPosition())
    {
        // Use cursor position for detection (unified position)
        CameraComponent->GetActorsInConeByMousePosition(
            CursorScreenPos,
            CharacterAttributes->OldManDetectionData,
            UGlobalTagName::Tag_BeDetcedItem,
            OutActors,
            OutDistances,
            OutAngles
        );
    }
    else
    {
        // Regular mode: use camera position and direction
        CameraComponent->GetActorsInCone(
            CharacterAttributes->OldManDetectionData,
            UGlobalTagName::Tag_BeDetcedItem,
            OutActors,
            OutDistances,
            OutAngles
        );
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
    PlayerCurHealth = 1.0f;

    // Initialize variables
    bIsRunning = false;
    hasIntoDoubleJump = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;
    bHasJumpInput = false;
    bHasAttackInput = false;
    bInPullState = false;

    // Landing detection improvements
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
    // Initialize camera animation component
    if (CameraAnimationComponent && CameraComponent)
    {
        CameraAnimationComponent->InitializeCameraAnimation(CameraComponent, this);
    }
}

void AOldManCharacter::InitializeEvent()
{
    UMonoManager::GetInstance()->SetInterval(0.1f, this, &AOldManCharacter::CheckPullItem);

    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, bool>(UGlobalEventName::Key_Player_OnChangeGrivity, this, &AOldManCharacter::ChangeSlopeState);

    UMyEventManager::GetInstance()->RegisterCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), this, &AOldManCharacter::UpdateInputActive);

    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, bool, FVector, FRotator>(UGlobalEventName::Key_Player_OnRespawn, this, &AOldManCharacter::OnPlayerRespawn);

    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, EHardwareDevicePrimaryType>(UGlobalEventName::Key_Input_InputDeviceChanged, this, &AOldManCharacter::OnInputDeviceChanged);
    if (AOldManPersonPlayerController* PC = GetOldManController())
    {
        EHardwareDevicePrimaryType CurrentDevice = PC->GetCurrentHardwareDeviceType();
        // Call handler to apply corresponding attributes
        OnInputDeviceChanged(CurrentDevice);
    }
}
#pragma endregion

#pragma region Item Fun
void AOldManCharacter::FireBullet(AActor* actor)
{
    // Spawn bullet
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

AOldManPullItemBase* AOldManCharacter::TryGetPullItem()
{
    AOldManPersonPlayerController* PC = GetOldManController();
    if (!PC) return nullptr;

    // Get effective cursor screen position
    FVector2D CursorScreenPos = PC->GetEffectiveCursorScreenPosition();

    // Convert screen coordinates to world ray
    FVector WorldOrigin, WorldDirection;
    if (UGameplayStatics::DeprojectScreenToWorld(PC, CursorScreenPos, WorldOrigin, WorldDirection))
    {
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.bTraceComplex = true;
        QueryParams.AddIgnoredActor(this);

        FVector TraceEnd = WorldOrigin + WorldDirection * 10000.0f;

        if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldOrigin, TraceEnd, ECC_Visibility, QueryParams))
        {
            if (AOldManPullItemBase* HitActor = Cast<AOldManPullItemBase>(HitResult.GetActor()))
            {
                return HitActor;
            }
        }
    }

    return nullptr;
}

void AOldManCharacter::CheckPullItem()
{
    if (AOldManPullItemBase * HitActor = TryGetPullItem())
    {
        curOldManPullItem = HitActor;
        curOldManPullItem->OnBeChecked();
    }
    else if (curOldManPullItem)
    {
        if (!curOldManPullItem->bIsBeingDragged)
        {
            curOldManPullItem->OnDismissChecked();
            curOldManPullItem = nullptr;
        }
    };
}

// ========== Modify StartRightMousePull to use unified cursor position ==========
void AOldManCharacter::StartRightMousePull()
{
    if (!GetIfCouldPullItem())
    {
        return;
    }

    if (AOldManPullItemBase* HitActor = TryGetPullItem())
    {
        SetPullItemState(true);
        HitActor->StartDragging();
        curOldManPullItem = HitActor;

        // Draw hit point
        DrawDebugSphere(GetWorld(), HitActor->GetActorLocation(), 15.0f, 12, FColor::Magenta, false, 5.0f, 0, 3.0f);
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
    // Mouse input
    float MouseXInput = mouseDelta.X;
    float MouseYInput = mouseDelta.Y;

    // Handle dragging
    if (curOldManPullItem && bInPullState)
    {
        APlayerCameraManager* CameraManager = GetOldManController()->PlayerCameraManager;
        if (CameraManager && (FMath::Abs(MouseXInput) > MinMovementThreshold || FMath::Abs(MouseYInput) > MinMovementThreshold))
        {
            FVector CameraLocation = CameraManager->GetCameraLocation();
            FRotator CameraRotation = CameraManager->GetCameraRotation();

            // Get camera direction vectors
            FVector CameraForward = CameraRotation.Vector();
            FVector CameraRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
            FVector CameraUp = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Z);

            // Build movement direction based on mouse input
            FVector ViewMovementDirection = (CameraRight * MouseXInput + CameraUp * MouseYInput).GetSafeNormal();

            // Compute movement intensity
            float MovementIntensity = FVector2D(MouseXInput, MouseYInput).Size() * DragSensitivity;

            // Apply movement
            curOldManPullItem->HandleMouseData(ViewMovementDirection, MovementIntensity);

            // Reset mouse input
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
void AOldManCharacter::OnPlayerRespawn(bool IfWaitInput, FVector ReBornPosition, FRotator ReBornRotation)
{
    InitializeParam();
    PlayerRebornData.IfWaitInput = IfWaitInput;
    PlayerRebornData.ReBornPosition = ReBornPosition;
    PlayerRebornData.ReBornRotation = ReBornRotation;
    UStateMachineManager* StateMachineManager = UStateMachineManager::GetStateMachineManager();
    if (StateMachineManager)
    {
        StateMachine->InitializeWithState(UOldManRebornState::StaticClass(), this);
    }
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