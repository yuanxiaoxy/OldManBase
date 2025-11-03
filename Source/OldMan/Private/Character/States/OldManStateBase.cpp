#include "Character/States/OldManStateBase.h"
#include "Character/OldManCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EventManager/MyEventManager.h"
#include "Character/States/OldManDeadState.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManOnSlopeState.h"

void UOldManStateBase::Enter()
{
    UE_LOG(LogTemp, Display, TEXT("%s : Enter"), *this->GetName());

    // 设置转换规则
    SetupTransitionRules();
    InPatchEvents();
}

void UOldManStateBase::Exit()
{
    UE_LOG(LogTemp, Display, TEXT("%s : Exit"), *this->GetName());

    // 清除缓存和转换规则
    CachedOldManCharacter = nullptr;
    TransitionRules.Empty();

    OutPatchEvents();
}

void UOldManStateBase::Update(float DeltaTime)
{
    CheckAllTransitions();
}

void UOldManStateBase::SetupTransitionRules()
{
    // 基类提供通用转换规则（死亡、斜坡、下落优先级最高）
    ADD_TRANSITION(UOldManDeadState, CheckDeathCondition);
    ADD_TRANSITION(UOldManOnSlopeState, CheckOnSlopeCondition);
    ADD_TRANSITION(UOldManFallingState, CheckFallingCondition);
}

void UOldManStateBase::AddTransitionRule(TSubclassOf<UStateBase> TargetState, FStateTransitionCondition Condition, const FString& DebugName)
{
    FStateTransitionRule Rule;
    Rule.TargetState = TargetState;
    Rule.Condition = Condition;
    Rule.DebugName = DebugName;
    TransitionRules.Add(Rule);
}

void UOldManStateBase::CheckAllTransitions()
{
    for (const FStateTransitionRule& Rule : TransitionRules)
    {
        if (Rule.Condition.IsBound() && Rule.Condition.Execute())
        {
            UE_LOG(LogTemp, Verbose, TEXT("Transition from %s to %s by condition: %s"),
                *GetName(),
                *Rule.TargetState->GetName(),
                *Rule.DebugName);

            CheckTransition(Rule.TargetState);
            return;
        }
    }
}

void UOldManStateBase::HandleMovement(float DeltaTime)
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

void UOldManStateBase::HandleMovementInAir(float DeltaTime)
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
        else if (!HasMovementInput() && GetCharacterMovement())//在空中没有移动输入
        {
            // 存储x，y值 插值运算
            int x = FMath::Lerp(GetCharacterMovement()->Velocity.X, 0, DeltaTime * Character->CharacterAttributes->SpeedChangeRateInAir);
            int y = FMath::Lerp(GetCharacterMovement()->Velocity.Y, 0, DeltaTime * Character->CharacterAttributes->SpeedChangeRateInAir);

            GetCharacterMovement()->Velocity.X = x;
            GetCharacterMovement()->Velocity.Y = y;
        }
    }
}

void UOldManStateBase::HandleRotation(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (HasMovementInput())
        {
            FVector MovementDirection = Character->GetMovementDirectionFromCamera();
            Character->UpdateCharacterRotation(DeltaTime, MovementDirection);
        }
    }
}

void UOldManStateBase::ApplyMovement(const FVector& Direction, float Speed)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
        {
            MovementComp->MaxWalkSpeed = Speed;
            Character->AddMovementInput(Direction);
        }
    }
}

void UOldManStateBase::Jump()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (Character->IsAlive())
        {
            Character->Jump();
        }
    }
}

bool UOldManStateBase::CheckDeathCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && !Character->IsAlive();
}

bool UOldManStateBase::CheckFallingCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return false;

    // 使用更可靠的检测方法
    bool bIsFalling = Character->IsFalling();

    // 调试日志
    if (bIsFalling)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Character is falling"));
    }

    return bIsFalling;
}

bool UOldManStateBase::CheckJumpCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bHasJumpInput;
}

bool UOldManStateBase::CheckAttackCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bHasAttackInput && Character->CanAttack();
}

bool UOldManStateBase::CheckPullItemStateCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bInCanPullState;
}

bool UOldManStateBase::CheckOnSlopeCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bIsOnSlope;
}

void UOldManStateBase::ResetJumpInput(bool jumpInputActive)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (Character)
    {
        Character->bHasJumpInput = jumpInputActive;
    }
}

AOldManCharacter* UOldManStateBase::GetOldManCharacter()
{
    // 缓存角色指针
    if (!CachedOldManCharacter)
    {
        CachedOldManCharacter = Cast<AOldManCharacter>(Owner.GetObject());
    }

    return CachedOldManCharacter;
}

UCharacterMovementComponent* UOldManStateBase::GetCharacterMovement()
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        return Character->GetCharacterMovement();
    }
    return nullptr;
}

bool UOldManStateBase::HasMovementInput()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->HasMovementInput();
}

bool UOldManStateBase::HasJumpInput()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bHasJumpInput;
}

bool UOldManStateBase::HasAttackInput()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bHasAttackInput;
}

bool UOldManStateBase::IsRunning()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bIsRunning;
}

void UOldManStateBase::InPatchEvents()
{
    // 子类可以重写
}

void UOldManStateBase::OutPatchEvents()
{
    // 子类可以重写
}