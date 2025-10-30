#include "Character/OldManMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

UOldManMovementComponent::UOldManMovementComponent()
{
    DefaultGravityDirection = FVector(0, 0, -1);
    CurrentGravityDirection = DefaultGravityDirection;
    TargetGravityDirection = DefaultGravityDirection;
    CurrentSurfaceNormal = DefaultGravityDirection;
    GravityStrength = 980.0f;
    bUseCustomGravity = false;
    GravityTransitionSpeed = 5.0f;

    SetMovementMode(MOVE_Custom, 0);
}

void UOldManMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bUseCustomGravity)
    {
        // 更新表面法线
        UpdateSurfaceNormal();

        // 平滑过渡重力方向
        if (!CurrentGravityDirection.Equals(TargetGravityDirection, 0.01f))
        {
            CurrentGravityDirection = FMath::VInterpTo(CurrentGravityDirection, TargetGravityDirection, DeltaTime, GravityTransitionSpeed);
            CurrentGravityDirection.Normalize();
        }

        UpdateCharacterOrientation();
    }
}

void UOldManMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME)
    {
        return;
    }

    // 保存旧速度用于计算加速度
    const FVector OldVelocity = Velocity;

    // 应用重力
    ApplyCustomGravity(deltaTime);

    // 应用摩擦力
    if (!Velocity.IsZero())
    {
        const float Friction = 0.2f;
        Velocity = Velocity * (1 - Friction * deltaTime);

        if (Velocity.Dot(OldVelocity) <= 0.0f)
        {
            Velocity = FVector::ZeroVector;
        }
    }

    // 移动角色
    FVector Delta = Velocity * deltaTime;
    if (!Delta.IsNearlyZero())
    {
        FHitResult Hit;
        SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentRotation(), true, Hit);

        if (Hit.IsValidBlockingHit())
        {
            SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
        }
    }

    // 更新加速度
    Acceleration = (Velocity - OldVelocity) / deltaTime;
}

void UOldManMovementComponent::MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDown)
{
    // 先调用父类实现
    Super::MoveAlongFloor(InVelocity, DeltaSeconds, OutStepDown);

    // 更新表面法线
    UpdateSurfaceNormal();
}

void UOldManMovementComponent::UpdateSurfaceNormal()
{
    if (!CharacterOwner || !UpdatedComponent)
    {
        return;
    }

    FVector NewSurfaceNormal = FindSurfaceNormal();

    if (!NewSurfaceNormal.IsZero())
    {
        CurrentSurfaceNormal = NewSurfaceNormal;
    }
}

FVector UOldManMovementComponent::FindSurfaceNormal() const
{
    if (!CharacterOwner || !UpdatedComponent)
    {
        return FVector::ZeroVector;
    }

    const float TraceDistance = 50.0f;
    const FVector Start = UpdatedComponent->GetComponentLocation();

    // 在重力方向上进行向下检测
    FVector TraceDirection = bUseCustomGravity ? CurrentGravityDirection : DefaultGravityDirection;
    FVector End = Start + TraceDirection * TraceDistance;

    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(CharacterOwner);

    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, CollisionParams))
    {
        return Hit.Normal;
    }

    // 如果没有命中，返回默认重力方向
    return bUseCustomGravity ? CurrentGravityDirection : DefaultGravityDirection;
}

void UOldManMovementComponent::ApplyCustomGravity(float DeltaTime)
{
    if (bUseCustomGravity)
    {
        // 计算重力加速度
        FVector GravityAcceleration = CurrentGravityDirection * GravityStrength;

        // 应用重力到速度
        Velocity += GravityAcceleration * DeltaTime;

        // 地面检测和响应
        FHitResult Hit;
        FVector TraceStart = UpdatedComponent->GetComponentLocation();

        // 修复：正确获取胶囊体半高
        float CapsuleHalfHeight = 0.0f;
        if (CharacterOwner && CharacterOwner->GetCapsuleComponent())
        {
            CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        }
        else
        {
            CapsuleHalfHeight = 88.0f; // 默认值
        }

        FVector TraceEnd = TraceStart + CurrentGravityDirection * (CapsuleHalfHeight + 2.0f);

        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(GetOwner());

        if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, CollisionParams))
        {
            // 在地面上，防止穿过地面
            float VerticalSpeed = FVector::DotProduct(Velocity, CurrentGravityDirection);
            if (VerticalSpeed < 0)
            {
                Velocity -= CurrentGravityDirection * VerticalSpeed;
            }
        }
    }
}

void UOldManMovementComponent::UpdateCharacterOrientation()
{
    if (ACharacter* OwnerCharacter = GetCharacterOwner())
    {
        // 计算新的前方向（保持角色原本的前方向，但投影到新的平面上）
        FVector CurrentForward = OwnerCharacter->GetActorForwardVector();
        FVector NewUp = -CurrentGravityDirection;

        // 将当前前方向投影到新的水平面
        FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp);

        // 如果投影后长度接近0，使用一个默认的前方向
        if (NewForward.IsNearlyZero())
        {
            NewForward = FVector::VectorPlaneProject(FVector(1, 0, 0), NewUp);
        }

        NewForward.Normalize();

        // 计算右方向
        FVector NewRight = FVector::CrossProduct(NewUp, NewForward);
        NewRight.Normalize();

        // 确保正交
        NewForward = FVector::CrossProduct(NewRight, NewUp);
        NewForward.Normalize();

        // 设置新的旋转
        FRotator NewRotation = FRotationMatrix::MakeFromXY(NewForward, NewRight).Rotator();
        OwnerCharacter->SetActorRotation(NewRotation);
    }
}

// 由于父类有 SetGravityDirection，我们使用不同的函数名
void UOldManMovementComponent::SetCustomGravityDirection(FVector NewGravityDirection, bool bInstant)
{
    NewGravityDirection.Normalize();
    TargetGravityDirection = NewGravityDirection;
    bUseCustomGravity = true;

    if (bInstant)
    {
        CurrentGravityDirection = TargetGravityDirection;
        UpdateCharacterOrientation();
    }
}

void UOldManMovementComponent::SetGravityFromSurfaceNormal(FVector SurfaceNormal, bool bInstant)
{
    // 使用表面法线的反方向作为重力方向
    FVector NewGravityDirection = -SurfaceNormal;
    SetCustomGravityDirection(NewGravityDirection, bInstant);
}

void UOldManMovementComponent::ResetCustomGravityDirection(bool bInstant)
{
    TargetGravityDirection = DefaultGravityDirection;

    if (bInstant)
    {
        CurrentGravityDirection = TargetGravityDirection;
        bUseCustomGravity = false;
        UpdateCharacterOrientation();
    }
}