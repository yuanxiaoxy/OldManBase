#include "Character/States/CableState/OldManOnCableHorizontalJumpState.h"
#include "Character/States/CableState/OldManOnCableMoveState.h"
#include "Character/OldManCharacter.h"
#include "Components/CapsuleComponent.h"

void UOldManOnCableHorizontalJumpState::Enter()
{
    Super::Enter();

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->SetMovementMode(MOVE_Custom);
        Movement->GravityScale = 0.0f;
        Movement->Velocity = FVector::ZeroVector;
    }

    JumpProgress = 0.0f;

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 继承当前滑索的移动方向
        bool bTargetMoveForward = Character->GetCableMoveDirectionSign() > 0;
        Character->SetCurrentCableWithDirection(Character->NextCable, bTargetMoveForward);
        CurrentCable = Character->NextCable;

        Character->PlayOnCableHoriaontalJumpAnimation(Character->IsLeftCable);

        JumpStartPosition = Character->GetActorLocation();

        if (CurrentCable)
        {
            CurrentCableDistance = CurrentCable->FindNearestDistanceAlongSpline(
                JumpStartPosition + Character->GetActorForwardVector() * Character->CharacterAttributes->HorizontalJumpForwardOffset);
            JumpTargetPosition = CurrentCable->GetPositionAtDistance(CurrentCableDistance);
            JumpTargetPosition = CalculateCharacterPositionOnCable(JumpTargetPosition);
        }

        if (Character->IsLeftCable)
            SetPlayerCurMoveState(EPlayerBaseMoveState::LeftHorizontalJump);
        else
            SetPlayerCurMoveState(EPlayerBaseMoveState::RightHorizontalJump);
    }
}

void UOldManOnCableHorizontalJumpState::Exit()
{
    Super::Exit();
}

void UOldManOnCableHorizontalJumpState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    HandleHorizontalJump(DeltaTime);
}

void UOldManOnCableHorizontalJumpState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_LAMBDA_TRANSITION(UOldManOnCableMoveState, [this]() {
        return JumpProgress >= 1.0f;
        }, "CableMove");
}

void UOldManOnCableHorizontalJumpState::HandleHorizontalJump(float DeltaTime)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return;

    // Update jump progress
    JumpProgress += DeltaTime * GetLateralJumpSpeed();
    JumpProgress = FMath::Min(JumpProgress, 1.0f);

    // Calculate new position - using linear interpolation
    FVector NewPosition = FMath::Lerp(JumpStartPosition, JumpTargetPosition, JumpProgress);

    // Update character location
    Character->SetActorLocation(NewPosition);

    // If jump completed, switch to move state
    if (JumpProgress >= 1.0f)
    {
        // Ensure character is correctly positioned on the target cable
        FVector FinalPosition = CurrentCable->GetCharacterPositionOnCable(
            JumpTargetPosition,
            Character->GetCapsuleComponent()->GetScaledCapsuleRadius()
        );
        Character->SetActorLocation(FinalPosition);
        AlignCharacterWithCable(FinalPosition);
    }
}

float UOldManOnCableHorizontalJumpState::GetLateralJumpSpeed()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->CharacterAttributes ?
        Character->CharacterAttributes->HorizontalJumpSpeed : 600.0f; // Increased speed
}

bool UOldManOnCableHorizontalJumpState::InLeftOfTarget(const FVector& Forward, const FVector& StartPos, const FVector& TargetPos)
{
    // 1. 计算水平方向的目标向量（忽略高度）
    FVector ToTarget = (TargetPos - StartPos).GetSafeNormal2D();
    FVector Forward2D = Forward.GetSafeNormal2D(); // 确保水平归一化

    // 2. 计算叉积 Z 分量（左手坐标系：X前 Y右 Z上）
    float CrossZ = FVector::CrossProduct(Forward2D, ToTarget).Z;

    // 3. 判断左右
    if (CrossZ > KINDA_SMALL_NUMBER)
    {
        return false;
    }
    else if (CrossZ < -KINDA_SMALL_NUMBER)
    {
        return true;
    }
    
    return true;
}