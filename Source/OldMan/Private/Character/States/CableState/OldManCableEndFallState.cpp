#include "Character/States/CableState/OldManCableEndFallState.h"
#include "Character/OldManCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOldManCableEndFallState::Enter()
{
    Super::Enter();

    ApplyLaunchVelocity();
}

void UOldManCableEndFallState::ApplyLaunchVelocity()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // Reset character rotation to upright (optional)
    FRotator NewRotation = Character->GetActorRotation();
    NewRotation.Pitch = 0.0f;
    NewRotation.Roll = 0.0f;
    Character->SetActorRotation(NewRotation);

    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        // Ensure falling mode
        MovementComp->SetMovementMode(MOVE_Falling);

        // Calculate launch velocity (forward direction * speed)
        FVector LaunchDirection = Character->GetActorForwardVector();
        float LaunchSpeed = Character->CharacterAttributes->MoveSpeedInCable *
            Character->CharacterAttributes->MoveSpeedMutiInEndCable;
        FVector LaunchVelocity = LaunchDirection * LaunchSpeed;

        // Apply velocity
        MovementComp->Velocity = LaunchVelocity;
        Character->LaunchCharacter(LaunchVelocity, false, true);

        UE_LOG(LogTemp, Log, TEXT("CableEndFallState: Launched with velocity %s"), *LaunchVelocity.ToString());
    }
}