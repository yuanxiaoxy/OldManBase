#include "Character/States/CableState/OldManOnCableExitState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/CableState/OldManOnCableFallState.h"
#include "GameFramework/CharacterMovementComponent.h"

void UOldManOnCableExitState::Enter()
{
    Super::Enter();

    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // Launch character from cable end (sets falling mode and initial velocity)
    LaunchCharacterAtCableEnd();

    // Immediately transition to falling state to handle further logic
    if (Character->StateMachine)
    {
        Character->StateMachine->ChangeState(UOldManOnCableFallState::StaticClass());
    }

    UE_LOG(LogTemp, Log, TEXT("ExitState entered: launched from cable end, switching to FallState"));
}

void UOldManOnCableExitState::Exit()
{
    Super::Exit();
}

void UOldManOnCableExitState::LaunchCharacterAtCableEnd()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    // Reset character rotation to upright
    FRotator NewRotation = Character->GetActorRotation();
    NewRotation.Pitch = 0.0f;
    NewRotation.Roll = 0.0f;
    Character->SetActorRotation(NewRotation);

    UCharacterMovementComponent* MovementComp = GetCharacterMovement();
    if (MovementComp)
    {
        MovementComp->SetMovementMode(MOVE_Falling);

        // 获取发射方向：使用角色当前面朝方向（已根据移动方向正确设置）
        FVector LaunchDirection = Character->GetActorForwardVector();
        float LaunchSpeed = Character->CharacterAttributes->MoveSpeedInCable *
            Character->CharacterAttributes->MoveSpeedMutiInEndCable;
        FVector LaunchVelocity = LaunchDirection * LaunchSpeed;

        MovementComp->Velocity = LaunchVelocity;
        Character->LaunchCharacter(LaunchVelocity, false, true);

        UE_LOG(LogTemp, Log, TEXT("ExitState: Launching with velocity %s"), *LaunchVelocity.ToString());
    }
}

FVector UOldManOnCableExitState::GetLaunchDirection()
{
    // 此函数可能不再需要，但保留以防其他地方调用
    AOldManCharacter* Character = GetOldManCharacter();
    return Character ? Character->GetActorForwardVector() : FVector::ForwardVector;
}