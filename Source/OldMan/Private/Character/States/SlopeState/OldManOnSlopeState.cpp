#include "Character/States/SlopeState/OldManOnSlopeState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/SlopeState/OldManFadeOutSlopeState.h"

void UOldManOnSlopeState::Enter()
{
    Super::Enter();
}

void UOldManOnSlopeState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
    HandleRotation(DeltaTime);
}


void UOldManOnSlopeState::HandleMovement(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        targetSpeed = Character->CharacterAttributes->MoveSpeedInSlope;;
        if (Character->MovementInputVector.X > 0 && GetCharacterMovement())
        {
            targetSpeed *= Character->CharacterAttributes->MoveSpeedMultiInSlope;
        }

        FVector tempVector = GetCharacterMovement()->Velocity;
        float Speed = FMath::Lerp(tempVector.Size(), targetSpeed,
            DeltaTime * Character->CharacterAttributes->SpeedChangeRate);

        ApplyMovement(Character->GetActorForwardVector(), Speed);
    }
}

void UOldManOnSlopeState::HandleRotation(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        //FVector MovementDirection = Character->GetMovementDirectionFromCamera();
        //Character->UpdateCharacterRotationByGravity(DeltaTime, MovementDirection);
        Character->UpdateCharacterRotationByGravity(DeltaTime);

        if (HasMovementInput())
        {
            FRotator NewRotation = FRotator::ZeroRotator;
            NewRotation.Yaw += Character->MovementInputVector.Y * 
                Character->CharacterAttributes->RotatorSpeedMultiInSlope;
            Character->AddActorLocalRotation(NewRotation);
        }
    }
}

void UOldManOnSlopeState::SetupTransitionRules()
{
    // 基类提供通用转换规则（死亡、斜坡、下落优先级最高）
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);
    ADD_LAMBDA_TRANSITION(UOldManFadeOutSlopeState,
        [this]() { return !CheckOnSlopeCondition(); },
        "FadeOutSlope");
}
