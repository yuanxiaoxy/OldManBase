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

    // Get parameters from character attributes
    FVector CharacterLocation = Character->GetActorLocation();
    FRotator CharacterRotation = Character->GetActorRotation();

    // Use character's local axes
    FVector CharacterRight = Character->GetActorRightVector();
    FVector CharacterForward = Character->GetActorForwardVector();
    FVector CharacterUp = Character->GetActorUpVector();

    if (HorizontalDir > 0)
    {
        RightCable = FindCableInBox(CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        Character->SetNextCable(RightCable.Cable, false);

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
        // Simple left-right box detection
        LeftCable = FindCableInBox(-CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        Character->SetNextCable(LeftCable.Cable, true);

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

    // Calculate box center in local space
    FVector BoxCenter = CharacterLocation + (Direction * Width * 0.5f);

    // Create detection box with different dimensions:
    // X: length (forward direction)
    // Y: width (lateral detection distance)
    // Z: height (vertical detection range)
    FVector BoxExtent = FVector(Length * 0.5f, Width * 0.5f, Height * 0.5f);

    // Box overlap detection - use character's rotation to align the box
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
        // Find the nearest cable
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
                    ClosestPosition = OverlapCable->GetCharacterPositionOnCable(
                        NearestPosition,
                        Character->GetCapsuleComponent()->GetScaledCapsuleRadius()
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

    // 如果移动方向是反向，则翻转角色的前向方向（绕上轴旋转180度）
    if (!Character->bCableMoveForward)
    {
        // 翻转Yaw角180度，使得角色面向电缆切线反方向
        CableRotation.Yaw += 180.0f;
        // 可选：同时翻转Pitch和Roll以保持正确姿态（根据你的需求决定是否启用）
        // CableRotation.Pitch = -CableRotation.Pitch;
        // CableRotation.Roll = -CableRotation.Roll;
    }

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

    // 获取角色在电缆上的最近点
    FVector NearestPoint = CurrentCable->FindNearestPosition(Character->GetActorLocation());
    // 电缆在该点的切线方向（从起点指向终点）
    FVector CableDir = CurrentCable->GetDirectionAtPosition(NearestPoint);
    CableDir.Normalize();

    // 决定移动方向的参考向量：优先使用移动输入方向，如果没有输入则使用角色面朝方向
    FVector ReferenceDir = Character->GetActorForwardVector(); // 默认面朝方向
    if (Character->HasMovementInput())
    {
        // 使用世界空间下的移动输入方向（已由相机旋转转换）
        FVector InputDir = Character->GetMovementDirectionFromCamera();
        if (!InputDir.IsNearlyZero())
        {
            ReferenceDir = InputDir;
        }
    }

    // 计算参考方向与电缆切线的点积
    float Dot = FVector::DotProduct(ReferenceDir, CableDir);

    // 如果点积为正，则向电缆终点方向移动（正向）；否则向起点方向移动（反向）
    bool bMoveForward = (Dot >= 0.0f);

    // 设置移动方向
    Character->bCableMoveForward = bMoveForward;

    UE_LOG(LogTemp, Log, TEXT("AutoDetermineCableDirection: ReferenceDir=%s, CableDir=%s, Dot=%.2f, MoveForward=%d"),
        *ReferenceDir.ToString(), *CableDir.ToString(), Dot, bMoveForward);
}