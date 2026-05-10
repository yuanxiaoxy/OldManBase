#include "Character/States/SlopeState/OldManOnSlopeState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/SlopeState/OldManFadeOutSlopeState.h"
#include "Character/States/CableState/OldManOnCableMoveState.h"    
#include "Components/BoxComponent.h"

void UOldManOnSlopeState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 绑定重叠事件
        Character->CableDetectionBox->OnComponentBeginOverlap.AddDynamic(this, &UOldManOnSlopeState::OnCableDetectionBeginOverlap);
        Character->CableDetectionBox->OnComponentEndOverlap.AddDynamic(this, &UOldManOnSlopeState::OnCableDetectionEndOverlap);
    }
}

void UOldManOnSlopeState::Exit()
{
    Super::Exit();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        Character->CableDetectionBox->OnComponentBeginOverlap.RemoveAll(this);
        Character->CableDetectionBox->OnComponentEndOverlap.RemoveAll(this);
    }
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
            Character->AddActorLocalRotation(DeltaTime * NewRotation);
        }
    }
}

void UOldManOnSlopeState::SetGravityScale()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->GravityScale = Character->CharacterAttributes->GravityInSlope;
    }
}

void UOldManOnSlopeState::SetupTransitionRules()
{
    // 基类提供通用转换规则（死亡、斜坡、下落优先级最高）
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);
    ADD_LAMBDA_TRANSITION(UOldManFadeOutSlopeState,
        [this]() { return !CheckOnSlopeCondition(); },
        "FadeOutSlope");
    // 检查是否存在钢索
    ADD_LAMBDA_TRANSITION(UOldManFadeOutSlopeState,
        [this]() {
            AOldManCharacter* Character = GetOldManCharacter();
            return Character && Character->HasCable();
        },
        "OnCable");
}

void UOldManOnSlopeState::SetPlayerCurActionState()
{
    if (GetOldManCharacter())
    {
        GetOldManCharacter()->SetPlayerCurActionState(EPlayerActionState::OnSlope);
    }
}
