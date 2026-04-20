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

    // 直接完成跳跃，不需要插值动画
    JumpProgress = 1.0f;

    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        // 继承当前滑索的移动方向（或根据目标电缆重新计算）
        bool bTargetMoveForward = Character->GetCableMoveDirectionSign() > 0;
        Character->SetCurrentCableWithDirection(Character->NextCable, bTargetMoveForward);
        CurrentCable = Character->NextCable;

        Character->PlayOnCableHoriaontalJumpAnimation(Character->IsLeftCable);

        // ✅ 直接使用检测时计算好的目标位置（已使用半高偏移，无需再次偏移）
        JumpTargetPosition = Character->NextCableJumpPosition;

        // 瞬间设置到目标位置
        Character->SetActorLocation(JumpTargetPosition);

        // 保持原有的旋转对齐逻辑（不做任何修改）
        AlignCharacterWithCable(JumpTargetPosition);

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
    // 无需插值，空实现
}

void UOldManOnCableHorizontalJumpState::SetupTransitionRules()
{
    Super::SetupTransitionRules();

    // 立即转换到移动状态
    ADD_LAMBDA_TRANSITION(UOldManOnCableMoveState, [this]() {
        return JumpProgress >= 1.0f;
        }, "CableMove");
}