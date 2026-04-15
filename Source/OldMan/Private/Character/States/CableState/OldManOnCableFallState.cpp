#include "Character/States/CableState/OldManOnCableFallState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/CableState/OldManOnCableDoubleJumpState.h"
#include "Character/States/CableState/OldManOnCableMoveState.h"
#include "Character/States/CableState/OldManOnCableHorizontalJumpState.h"
#include "Components/BoxComponent.h"

void UOldManOnCableFallState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // Ensure movement component is properly configured
        if (UCharacterMovementComponent* Movement = GetCharacterMovement())
        {
            // Explicitly set falling mode
            Movement->SetMovementMode(MOVE_Falling);
            Movement->GravityScale = Character->CharacterAttributes->GravityInCable;
        }

        targetSpeed = Character->CharacterAttributes->MoveSpeedOnAirInCable;

        // Bind overlap events for cable detection
        Character->CableDetectionBox->OnComponentBeginOverlap.AddDynamic(this, &UOldManOnCableFallState::OnCableDetectionBeginOverlap);
        Character->CableDetectionBox->OnComponentEndOverlap.AddDynamic(this, &UOldManOnCableFallState::OnCableDetectionEndOverlap);

        // Reset nearby cable detection results
        LeftCable = FCableDetectionResult();
        RightCable = FCableDetectionResult();

        SetPlayerCurMoveState(EPlayerBaseMoveState::Fall);

        UE_LOG(LogTemp, Log, TEXT("FallState entered"));
    }
}

void UOldManOnCableFallState::Exit()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        Character->CableDetectionBox->OnComponentBeginOverlap.RemoveAll(this);
        Character->CableDetectionBox->OnComponentEndOverlap.RemoveAll(this);
    }

    Super::Exit();
}

void UOldManOnCableFallState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    // Handle air movement (from base class)
    HandleMovementOnCableInAir(DeltaTime);

    // Check for horizontal input to detect nearby cables
    HandleCableInput(DeltaTime);
}

void UOldManOnCableFallState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // Transition to OnCableMoveState if character is on a cable (detected by overlap)
    ADD_LAMBDA_TRANSITION(UOldManOnCableMoveState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            return Character && Character->HasCable();
        },
        "OnCable");

    // Transition to LandState when grounded
    ADD_LAMBDA_TRANSITION(UOldManLandState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            if (!Character) return false;

            bool bIsActuallyGrounded = Character->IsActuallyGrounded();
            float TimeSinceLanding = Character->GetTimeSinceLastLanding();

            // Here we could add visual effects for cable landing

            return !CheckFallingCondition() && (bIsActuallyGrounded || TimeSinceLanding < 0.3f);
        },
        "Landing");

    // Transition to DoubleJumpState if double jump input is available
    ADD_LAMBDA_TRANSITION(UOldManOnCableDoubleJumpState,
        [this]() {
            return HasJumpInput() && GetOldManCharacter() && GetOldManCharacter()->CanDoubleJump();
        },
        "DoubleJump");

    // NEW: Transition to HorizontalJumpState if jump input and a valid next cable exists
    ADD_LAMBDA_TRANSITION(UOldManOnCableHorizontalJumpState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            return Character && Character->bHasJumpInput && Character->NextCable != nullptr;
        },
        "HorizontalJump");
}

void UOldManOnCableFallState::SetGravityScale()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Falling);
        Movement->GravityScale = Character->CharacterAttributes->GravityInCable;
    }
}

void UOldManOnCableFallState::HandleCableInput(float DeltaTime)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // Get horizontal input (Y axis: left/right)
    float InputY = Character->HasMovementInput() ? Character->MovementInputVector.Y : 0.0f;

    // Update nearby cable detection based on input direction
    if (FMath::Abs(InputY) > 0.1f)
    {
        UpdateNearbyCableDetection(InputY);
    }
    else
    {
        // No horizontal input, clear next cable
        Character->SetNextCable(nullptr, false);
    }
}

void UOldManOnCableFallState::OnCableDetectionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AOldManCableBase* Cable = Cast<AOldManCableBase>(OtherActor);
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Cable || !Character) return;

    if (Character->CurrentCable == Cable) return;

    Character->SetCurrentCable(Cable);
}

void UOldManOnCableFallState::OnCableDetectionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // Optional: handle end overlap
}