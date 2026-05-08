#include "Character/States/CableState/OldManOnCableState.h"
#include "Character/OldManCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManJumpingState.h"
#include "Engine/OverlapResult.h"
#include "EffectManager/EffectManager.h"

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
    if (!Character || !CurrentCable)
    {
        // 角色或当前缆绳无效时，隐藏特效
        HideDetectionEffects();
        return;
    }

    // 方向为零：不进行检测，隐藏特效
    if (FMath::IsNearlyZero(HorizontalDir))
    {
        HideDetectionEffects();
        return;
    }

    FVector CharacterLocation = Character->GetActorLocation();
    FRotator CharacterRotation = Character->GetActorRotation();
    FVector CharacterRight = Character->GetActorRightVector();

    bool bDetected = false;
    FVector TargetPosition = FVector::ZeroVector;
    FCableDetectionResult TargetCable = FCableDetectionResult();
    bool bIsLeft = false;

    if (HorizontalDir > 0)
    {
        // 检测右侧缆绳
        RightCable = FindCableInBox(CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        if (RightCable.Cable)
        {
            TargetCable = RightCable;
            TargetPosition = RightCable.Position;
            bIsLeft = false;
            bDetected = true;
        }
    }
    else if (HorizontalDir < 0)
    {
        // 检测左侧缆绳
        LeftCable = FindCableInBox(-CharacterRight, LateralJumpDistance, DetectionHeight, DetectionLength);
        if (LeftCable.Cable)
        {
            TargetCable = LeftCable;
            TargetPosition = LeftCable.Position;
            bIsLeft = true;
            bDetected = true;
        }
    }

    if (bDetected)
    {
        // 更新角色的跳跃目标信息
        Character->SetNextCable(TargetCable.Cable, bIsLeft);
        Character->IsLeftCable = bIsLeft;
        Character->NextCableJumpPosition = TargetPosition;

        UEffectManager* EffectMgr = UEffectManager::GetInstance();
        if (!EffectMgr) return;

        // 创建或更新线条特效（附加到角色中心，每帧设置终点位置）
        if (!EffectMgr->GetEffectInstanceComponent(Line_Name))
        {
            Line_Name = EffectMgr->PlayEffectAtLocation("HorizontalLine", GetOldManCharacter()->GetLineEffectStartPos());
        }
        else
        {
            EffectMgr->SetEffectWorldLocation(Line_Name, GetOldManCharacter()->GetLineEffectStartPos());

            FVector Delta = TargetPosition - GetOldManCharacter()->GetLineEffectStartPos();
            EffectMgr->SetEffectVectorParameter(Line_Name, "Target", Delta);
        }

        // 创建或更新球体特效（位于目标点位置）
        if (!EffectMgr->GetEffectInstanceComponent(Sphere_Name))
        {
            Sphere_Name = EffectMgr->PlayEffectAtLocation("TargetPosSphere", TargetPosition);
        }
        else
        {
            EffectMgr->SetEffectWorldLocation(Sphere_Name, TargetPosition);
        }
    }
    else
    {
        // 未检测到有效缆绳，隐藏特效
        HideDetectionEffects();
    }
}

void UOldManOnCableState::HideDetectionEffects()
{
    UEffectManager* EffectMgr = UEffectManager::GetInstance();
    if (!EffectMgr) return;

    if (!Line_Name.IsNone())
    {
        EffectMgr->DestroyEffectInstance(Line_Name);
        Line_Name = NAME_None;
    }
    if (!Sphere_Name.IsNone())
    {
        EffectMgr->DestroyEffectInstance(Sphere_Name);
        Sphere_Name = NAME_None;
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
                //ApplyMovement(MovementDirection, targetSpeed);
            }
            else
            {
                ApplyMovement(Character->GetActorForwardVector(), targetSpeed);
            }

            // Handle rotation
            //HandleRotation(DeltaTime);
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