#include "Character/States/CableState/OldManOnCableMoveState.h"
#include "Character/OldManCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/States/CableState/OldManOnCableExitState.h"
#include "Character/States/CableState/OldManOnCableJumpState.h"
#include "Character/States/CableState/OldManOnCableHorizontalJumpState.h"
#include "GlobalTagName.h"
#include "AudioManager/AudioManager.h"

void UOldManOnCableMoveState::Enter()
{
    Super::Enter();

    // Reset nearby cable detection
    LeftCable = FCableDetectionResult();
    RightCable = FCableDetectionResult();

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        Character->PlayOnCableAnimation();

        // Reset jump input
        ResetJumpInput(false);
        CurrentCableDistance = Character->CurrentCable->FindNearestDistanceAlongSpline(Character->GetActorLocation());

        SetPlayerCurMoveState(EPlayerBaseMoveState::Walk);
        //玩家进入滑索事件
        if (CurrentCable)
        {
            CurrentCable->CharacterEnterCable(Character->bCableMoveForward);
        }

        if (CurrentCable->Tags.Find(UGlobalTagName::Tag_ElectricityCable) > -1)
        {
            MoveSoundName = "SFX_Cable_ElectricityMove";
        }
        else
        {
            MoveSoundName = "SFX_Cable_CommonMove";
        }
        UAudioManager::GetInstance()->PlaySound(Character, MoveSoundName);
    }
}

void UOldManOnCableMoveState::Exit()
{
    Super::Exit();

    AOldManCharacter* Character = GetOldManCharacter();
    //玩家退出滑索事件
    if (CurrentCable)
    {
        CurrentCable->CharacterExitCable(Character->bCableMoveForward);
    }

    if (Character)
    {
        Character->SetCurrentCable(nullptr);
    }

    UAudioManager::GetInstance()->StopSound(MoveSoundName);

    HideDetectionEffects();
}

void UOldManOnCableMoveState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    UpdateCableMovement(DeltaTime);
    HandleCableInput(DeltaTime);
}

void UOldManOnCableMoveState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    ADD_LAMBDA_TRANSITION(UOldManOnCableExitState, [this]() {
        AOldManCharacter* Character = GetOldManCharacter();
        if (!Character || !Character->CurrentCable) return false;
        float CableLength = Character->CurrentCable->GetCableLength();
        const float Epsilon = 1.0f; // 容差，确保反向时也能触发
        return CurrentCableDistance <= Epsilon || CurrentCableDistance >= CableLength - Epsilon;
        }, "NoCable");

    //ADD_LAMBDA_TRANSITION(UOldManOnCableJumpState, [this]() {
    //    AOldManCharacter* Character = GetOldManCharacter();
    //    return Character && Character->bHasJumpInput && !Character->NextCable;
    //    }, "Jump");

    ADD_LAMBDA_TRANSITION(UOldManOnCableHorizontalJumpState, [this]() {
        AOldManCharacter* Character = GetOldManCharacter();
        return Character && Character->bHasJumpInput && Character->NextCable;
        }, "HorizontalJump");
}

void UOldManOnCableMoveState::SetGravityScale()
{
    // Disable gravity and enable custom movement
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (Movement)
    {
        Movement->SetMovementMode(MOVE_Custom);
        Movement->GravityScale = 0.0f;
        Movement->Velocity = FVector::ZeroVector;
    }
}

void UOldManOnCableMoveState::UpdateCableMovement(float DeltaTime)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !Character->CurrentCable) return;

    float MoveSpeed = Character->CharacterAttributes->MoveSpeedInCable;
    float Direction = Character->GetCableMoveDirectionSign();

    // 如果是单向滑索，强制方向固定，不依赖于 bCableMoveForward（但已经由 AutoDetermineCableDirection 确定了）
    // 这里我们可以保留 Direction 的计算，因为单向滑索的 bCableMoveForward 在 AutoDetermineCableDirection 中会被正确设置。
    // 但我们仍需确保单向滑索不会因为玩家输入而改变方向，MoveAlongCable 已经处理了强制方向，所以这里使用 Direction 没问题。

    CurrentCableDistance += MoveSpeed * Direction * DeltaTime;
    CurrentCableDistance = FMath::Clamp(CurrentCableDistance, 0.0f, Character->CurrentCable->GetCableLength());

    FVector NewPosition = Character->CurrentCable->GetPositionAtDistance(CurrentCableDistance);
    FVector AdjustedPosition = CalculateCharacterPositionOnCable(NewPosition);
    Character->SetActorLocation(AdjustedPosition);
    AlignCharacterWithCable(NewPosition);
}
void UOldManOnCableMoveState::HandleCableInput(float DeltaTime)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    if (Character->HasMovementInput() && FMath::Abs(Character->MovementInputVector.Y) > 0.1f)
    {
        float InputVector = Character->MovementInputVector.Y;
        UpdateNearbyCableDetection(InputVector);
    }
    else
    {
        HideDetectionEffects();
        Character->SetNextCable(nullptr, false);
    }
}