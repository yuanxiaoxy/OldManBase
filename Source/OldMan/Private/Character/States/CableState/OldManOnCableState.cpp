#include "Character/States/CableState/OldManOnCableState.h"
#include "Character/OldManCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Engine/OverlapResult.h"

void UOldManOnCableState::Enter()
{
    Super::Enter();

    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    CurrentCable = Character->CurrentCable;
    LateralJumpDistance = Character->CharacterAttributes->HorizontalJumpDistance;
    DetectionLength = Character->CharacterAttributes->HorizontalJumpLength;
    DetectionHeight = Character->CharacterAttributes->HorizontalJumpHeight;

    CurrentCableDistance = 0.0f;

    // 自动计算移动方向
    AutoDetermineCableDirection();
}

void UOldManOnCableState::Update(float DeltaTime)
{
    Super::Update(DeltaTime);

    ApplyCableGravity(DeltaTime);
}

void UOldManOnCableState::Exit()
{
    Super::Exit();

    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;
    Character->OldManMovementComponent->SetGravityDirection(FVector::DownVector);
}

void UOldManOnCableState::SetupTransitionRules()
{
    Super::SetupTransitionRules();
}

void UOldManOnCableState::UpdateNearbyCableDetection(float HorizontalDir)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return;

    FVector CharacterLocation = Character->GetActorLocation();
    FRotator CharacterRotation = Character->GetActorRotation();
    FVector CharacterRight = Character->GetActorRightVector();

    if (HorizontalDir > 0)
    {
        RightCable = FindCableInBox(CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        Character->SetNextCable(RightCable.Cable, false);
        Character->IsLeftCable = false;   // 标记向右跳
        Character->NextCableJumpPosition = RightCable.Position;  // 保存目标位置

        UWorld* World = Character->GetWorld();
        if (World)
        {
            // Calculate box center and extent in local space
            FVector BoxExtent = FVector(DetectionLength * 0.5f, LateralJumpDistance * 0.5f, DetectionHeight * 0.5f);
            FVector RightBoxCenter = CharacterLocation + (CharacterRight * LateralJumpDistance * 0.5f);

            // Draw debug box using character's rotation
            DrawDebugBox(
                World,
                RightBoxCenter,
                BoxExtent,
                CharacterRotation.Quaternion(),
                FColor::Green,
                false,
                -1.0f,
                0,
                2.0f
            );

            // If cable detected, draw connection lines
            if (RightCable.Cable)
            {
                DrawDebugLine(World, CharacterLocation, RightCable.Position, FColor::Green, false, -1.0f, 0, 2.0f);
                DrawDebugSphere(World, RightCable.Position, 20.0f, 8, FColor::Green, false, -1.0f, 0, 2.0f);
            }
        }
    }
    else if (HorizontalDir < 0)
    {
        LeftCable = FindCableInBox(-CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        Character->SetNextCable(LeftCable.Cable, true);
        Character->IsLeftCable = true;    // 标记向左跳
        Character->NextCableJumpPosition = LeftCable.Position;   // 保存目标位置

        UWorld* World = Character->GetWorld();
        if (World)
        {
            FVector BoxExtent = FVector(DetectionLength * 0.5f, LateralJumpDistance * 0.5f, DetectionHeight * 0.5f);
            FVector LeftBoxCenter = CharacterLocation + (-CharacterRight * LateralJumpDistance * 0.5f);

            DrawDebugBox(
                World,
                LeftBoxCenter,
                BoxExtent,
                CharacterRotation.Quaternion(),
                FColor::Blue,
                false,
                -1.0f,
                0,
                2.0f
            );

            if (LeftCable.Cable)
            {
                DrawDebugLine(World, CharacterLocation, LeftCable.Position, FColor::Blue, false, -1.0f, 0, 2.0f);
                DrawDebugSphere(World, LeftCable.Position, 20.0f, 8, FColor::Blue, false, -1.0f, 0, 2.0f);
            }
        }
    }
}

FCableDetectionResult UOldManOnCableState::FindCableInBox(const FVector& Direction, float Width, float Height, float Length)
{
    FCableDetectionResult Result;
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return Result;

    FVector CharacterLocation = Character->GetActorLocation();
    FRotator CharacterRotation = Character->GetActorRotation();

    FVector BoxCenter = CharacterLocation + (Direction * Width * 0.5f);
    FVector BoxExtent = FVector(Length * 0.5f, Width * 0.5f, Height);

    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    QueryParams.AddIgnoredActor(CurrentCable);

    UWorld* World = Character->GetWorld();
    if (World && World->OverlapMultiByChannel(
        OverlapResults,
        BoxCenter,
        CharacterRotation.Quaternion(),
        ECC_WorldStatic,
        FCollisionShape::MakeBox(BoxExtent),
        QueryParams
    ))
    {
        float ClosestDistance = MAX_FLT;
        AOldManCableBase* ClosestCable = nullptr;
        FVector ClosestPosition = FVector::ZeroVector;
        float ClosestCableDistance = 0.0f;

        for (const FOverlapResult& OverlapResult : OverlapResults)
        {
            AOldManCableBase* OverlapCable = Cast<AOldManCableBase>(OverlapResult.GetActor());
            if (OverlapCable && OverlapCable != CurrentCable)
            {
                FVector NearestPosition = OverlapCable->FindNearestPosition(CharacterLocation);
                float Distance = FVector::Distance(CharacterLocation, NearestPosition);

                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestCable = OverlapCable;
                    // ✅ 修改：使用胶囊体半高，与移动状态保持一致
                    ClosestPosition = OverlapCable->GetCharacterPositionOnCable(
                        NearestPosition,
                        Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
                    );
                    ClosestCableDistance = OverlapCable->FindNearestDistanceAlongSpline(NearestPosition);
                }
            }
        }

        if (ClosestCable)
        {
            Result.Cable = ClosestCable;
            Result.Position = ClosestPosition;
            Result.Distance = ClosestDistance;
            Result.CableDistance = ClosestCableDistance;
        }
    }

    return Result;
}

FVector UOldManOnCableState::CalculateCharacterPositionOnCable(const FVector& WorldPosition)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return WorldPosition;

    return CurrentCable->GetCharacterPositionOnCable(
        WorldPosition,
        Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

void UOldManOnCableState::AlignCharacterWithCable(const FVector& WorldPosition)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return;

    // 获取电缆在该位置的原始变换（切线指向终点）
    FTransform CableTransform = CurrentCable->GetTransformAtPosition(WorldPosition);
    FRotator CableRotation = CableTransform.Rotator();

    // 只有双向滑索才根据移动方向翻转角色朝向
    if (CurrentCable->IsBidirectional() && !Character->bCableMoveForward)
    {
        // 翻转Yaw角180度，使得角色面向电缆切线反方向
        CableRotation.Yaw += 180.0f;
        // 如果需要，也可以翻转Pitch和Roll以保持正确姿态，视需求而定
    }
    // 单向滑索直接使用样条旋转，不做额外翻转

    Character->SetActorRotation(CableRotation);
}

void UOldManOnCableState::HandleMovementOnCableInAir(float DeltaTime)
{
    if (AOldManCharacter* Character = GetOldManCharacter())
    {
        if (GetCharacterMovement())
        {
            // Use the effective camera rotation to compute movement direction
            FVector MovementDirection = Character->GetMovementDirectionFromCamera();
            if (!MovementDirection.IsNearlyZero())
            {
                ApplyMovement(MovementDirection, targetSpeed);
            }
            else
            {
                ApplyMovement(Character->GetActorForwardVector(), targetSpeed);
            }

            // Handle rotation
            HandleRotation(DeltaTime);
        }
    }
}

void UOldManOnCableState::SetGravityScale()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character) return;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        Movement->SetMovementMode(MOVE_Custom);
        Movement->GravityScale = Character->CharacterAttributes->GravityInCable;
        Movement->Velocity = FVector::ZeroVector;
    }
}

void UOldManOnCableState::SetPlayerCurActionState()
{
    if (GetOldManCharacter())
    {
        GetOldManCharacter()->SetPlayerCurActionState(EPlayerActionState::OnCable);
    }
}

void UOldManOnCableState::ApplyCableGravity(float DeltaTime)
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return;

    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
    {
        // Get gravity direction perpendicular to cable (down along character's up vector)
        FVector GravityDirection = -Character->GetActorUpVector();

        // Calculate gravity acceleration (same magnitude as default gravity)
        float GravityAcceleration = GetWorld()->GetGravityZ();
        FVector GravityVector = GravityDirection * -GravityAcceleration;
        Character->OldManMovementComponent->SetGravityDirection(GravityVector.GetSafeNormal());

        DrawDebugDirectionalArrow(
            GetWorld(),
            Character->GetActorLocation(),
            Character->GetActorLocation() + GravityDirection * 200.0f,
            50.0f,
            FColor::Red,
            false,
            -1.0f,
            0,
            5.0f
        );
    }
}

void UOldManOnCableState::AutoDetermineCableDirection()
{
    AOldManCharacter* Character = GetOldManCharacter();
    if (!Character || !CurrentCable) return;

    // 如果是单向滑索，直接根据滑索的 bReverseMovementDirection 确定移动方向
    if (!CurrentCable->IsBidirectional())
    {
        // 单向滑索：固定方向，由滑索属性决定
        // bReverseMovementDirection = true 表示反向移动（向起点），false 表示正向移动（向终点）
        bool bMoveForward = !CurrentCable->bReverseMovementDirection;
        Character->bCableMoveForward = bMoveForward;
        return;
    }

    // 双向滑索：根据玩家朝向或输入方向自动确定
    FVector NearestPoint = CurrentCable->FindNearestPosition(Character->GetActorLocation());
    FVector CableDir = CurrentCable->GetDirectionAtPosition(NearestPoint);
    CableDir.Normalize();

    FVector ReferenceDir = Character->GetActorForwardVector();
    if (Character->HasMovementInput())
    {
        FVector InputDir = Character->GetMovementDirectionFromCamera();
        if (!InputDir.IsNearlyZero())
        {
            ReferenceDir = InputDir;
        }
    }

    float Dot = FVector::DotProduct(ReferenceDir, CableDir);
    Character->bCableMoveForward = (Dot >= 0.0f);
}