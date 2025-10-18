#include "Character/States/OldManStateBase.h"
#include "Character/OldManCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EventManager/MyEventManager.h"

void UOldManStateBase::Enter()
{
    UE_LOG(LogTemp, Display, TEXT("%s : Enter"), *this->GetName());

    InPatchEvents();
}

void UOldManStateBase::Exit()
{
    UE_LOG(LogTemp, Display, TEXT("%s : Exit"), *this->GetName());

    // �������
    CachedOldManCharacter = nullptr;

    OutPatchEvents();
}

void UOldManStateBase::Update(float DeltaTime)
{
    
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
                //ȡ��z��Ӱ��
                FVector tempVector = GetCharacterMovement()->Velocity;
                tempVector.Z = 0.0f;
                float Speed = FMath::Lerp(tempVector.Size(), targetSpeed,
                    DeltaTime * Character->CharacterAttributes->SpeedChangeRate);

                ApplyMovement(MovementDirection, Speed);

                // ������ת
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
                //ȡ��z��Ӱ��
                FVector tempVector = GetCharacterMovement()->Velocity;
                tempVector.Z = 0.0f;
                float Speed = FMath::Lerp(tempVector.Size(), targetSpeed,
                    DeltaTime * Character->CharacterAttributes->SpeedChangeRate);

                ApplyMovement(MovementDirection, Speed);

                // ������ת
                HandleRotation(DeltaTime);
            }
        }
        else if (!HasMovementInput() && GetCharacterMovement())//�ڿ���û���ƶ�����
        {
            //�洢x��yֵ ��ֵ����
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

    // ʹ�ø��ɿ��ļ�ⷽ��
    bool bIsFalling = Character->IsFalling();

    // ������־
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

bool UOldManStateBase::CheckPullItemCondition()
{
    AOldManCharacter* Character = GetOldManCharacter();
    return Character && Character->bHasPullItem;
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
    // �����ɫָ��
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
    //UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckJumpStatesTranisition, this, &UOldManStateBase::CheckJumpCondition);
    //UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckAttackStatesTranisition, this, &UOldManStateBase::CheckAttackCondition);
}

void UOldManStateBase::OutPatchEvents()
{
    //UMyEventManager::GetInstance()->RemoveCppEvent(Key_CheckJumpStatesTranisition);
    //UMyEventManager::GetInstance()->RemoveCppEvent(Key_CheckAttackStatesTranisition);
}