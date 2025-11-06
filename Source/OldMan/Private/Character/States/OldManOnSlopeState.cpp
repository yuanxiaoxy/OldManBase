#include "Character/States/OldManOnSlopeState.h"
#include "Character/OldManCharacter.h"

void UOldManOnSlopeState::Enter()
{
    Super::Enter();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 重置移动速度为行走速度
        if (GetCharacterMovement() && Character->CharacterAttributes)
        {
            GetCharacterMovement()->MaxWalkSpeed = Character->CharacterAttributes->MoveSpeedInSlope;
        }

        targetSpeed = 0.0f;

        // 调用移动动画事件
        Character->PlayOnSlopeAnimation();
    }
}

void UOldManOnSlopeState::Exit()
{
    Super::Exit();
}

void UOldManOnSlopeState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    HandleMovement(DeltaTime);
}


void UOldManOnSlopeState::HandleMovement(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (HasMovementInput() && GetCharacterMovement())
        {
            FVector MovementDirection = Character->GetMovementDirectionFromCamera();
            if (!MovementDirection.IsNearlyZero())
            {
                // 取消z轴影响
                FVector tempVector = GetCharacterMovement()->Velocity;
                tempVector.Z = 0.0f;
                float Speed = FMath::Lerp(tempVector.Size(), targetSpeed,
                    DeltaTime * Character->CharacterAttributes->SpeedChangeRate);

                ApplyMovement(MovementDirection, Speed);

                // 处理旋转
                HandleRotation(DeltaTime);
            }
        }
    }
}

void UOldManOnSlopeState::HandleRotation(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (HasMovementInput())
        {
            FVector MovementDirection = Character->GetMovementDirectionFromCamera();
            Character->UpdateCharacterRotation(DeltaTime, MovementDirection);
        }

        if (HasMovementInput())
        {
            FRotator NewRotation = FRotator::ZeroRotator;
            NewRotation.Yaw += Character->MovementInputVector.Y;
            //Character->SetActorRotation(NewRotation);
            Character->AddActorLocalRotation(NewRotation);
        }
    }
}

void UOldManOnSlopeState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    //// 添加Idle状态特有的转换规则
    //ADD_TRANSITION(UOldManJumpingState, CheckJumpCondition);
    //ADD_LAMBDA_TRANSITION(UOldManWalkingState,
    //    [this]() { return HasMovementInput(); },
    //    "MovementInput");
}