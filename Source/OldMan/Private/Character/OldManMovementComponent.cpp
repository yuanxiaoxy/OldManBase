#include "Character/OldManMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UOldManMovementComponent::UOldManMovementComponent()
{
    DefaultGravityDirection = FVector(0, 0, -1);
    CurrentGravityDirection = DefaultGravityDirection;
    TargetGravityDirection = DefaultGravityDirection;
    GravityStrength = 980.0f;
    bUseCustomGravity = false;
    GravityTransitionSpeed = 5.0f;
    RaycastDistance = 200.0f;
    RaycastChannel = ECC_WorldStatic;
    bEnableDebugDrawing = false;

    bLastRaycastHit = false;
    LastHitLocation = FVector::ZeroVector;
    LastHitNormal = FVector::ZeroVector;

    // 启用行走模式支持各种角度
    SetWalkableFloorZ(0.1f);

    // 禁用自动旋转，由我们手动控制
    bOrientRotationToMovement = false;
}

void UOldManMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    // 在父类处理前更新重力方向
    if (bUseCustomGravity)
    {
        // 平滑过渡重力方向
        if (!CurrentGravityDirection.Equals(TargetGravityDirection, 0.01f))
        {
            CurrentGravityDirection = FMath::VInterpTo(CurrentGravityDirection, TargetGravityDirection, DeltaTime, GravityTransitionSpeed);
            CurrentGravityDirection.Normalize();
        }

        // 始终应用自定义重力，而不仅仅在下落时
        ApplyCustomGravity(DeltaTime);

        // 更新角色朝向
        UpdateCharacterOrientation();
    }

    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UOldManMovementComponent::PhysicsRotation(float DeltaTime)
{
    // 在自定义重力下，我们手动处理旋转
    if (bUseCustomGravity)
    {
        // 不调用父类的旋转逻辑，避免冲突
        return;
    }

    Super::PhysicsRotation(DeltaTime);
}

void UOldManMovementComponent::ApplyCustomGravity(float DeltaTime)
{
    if (!bUseCustomGravity)
        return;

    // 始终应用自定义重力，确保角色紧贴表面
    FVector GravityVector = CurrentGravityDirection * GravityStrength * DeltaTime;

    // 应用重力到速度
    Velocity += GravityVector;

    // 如果在地面上，添加额外的向下的力以确保紧贴表面
    if (IsMovingOnGround())
    {
        // 添加额外的向下的力，确保角色紧贴斜面
        FVector AdditionalForce = CurrentGravityDirection * GravityStrength * 0.5f * DeltaTime;
        Velocity += AdditionalForce;
    }
}

bool UOldManMovementComponent::PerformRaycast(FVector& OutHitLocation, FVector& OutHitNormal)
{
    if (!CharacterOwner || !UpdatedComponent)
    {
        return false;
    }

    // 获取胶囊体信息
    float CapsuleRadius, CapsuleHalfHeight;
    CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

    FVector RayStart = UpdatedComponent->GetComponentLocation();

    // 使用当前重力方向作为射线方向
    FVector RayDirection = bUseCustomGravity ? CurrentGravityDirection : DefaultGravityDirection;
    FVector RayEnd = RayStart + RayDirection * RaycastDistance;

    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredActor(CharacterOwner);

    FHitResult Hit;
    bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        RayStart,
        RayEnd,
        FQuat::Identity,
        RaycastChannel,
        FCollisionShape::MakeSphere(CapsuleRadius * 0.8f),
        CollisionParams
    );

    // 调试绘制
    if (bEnableDebugDrawing && GetWorld()->IsGameWorld())
    {
        FColor DebugColor = bHit ? FColor::Green : FColor::Red;
        DrawDebugLine(GetWorld(), RayStart, RayEnd, DebugColor, false, 0, 0, 2.0f);

        if (bHit)
        {
            DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Yellow, false, 0, 0);
            DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * 50.0f, FColor::Blue, false, 0, 0, 2.0f);

            // 绘制重力方向
            DrawDebugLine(GetWorld(), RayStart, RayStart + CurrentGravityDirection * 100.0f, FColor::Magenta, false, 0, 0, 3.0f);
        }
    }

    if (bHit)
    {
        OutHitLocation = Hit.ImpactPoint;
        OutHitNormal = Hit.ImpactNormal;
        bLastRaycastHit = true;
        LastHitLocation = Hit.ImpactPoint;
        LastHitNormal = Hit.ImpactNormal;
    }
    else
    {
        bLastRaycastHit = false;
    }

    return bHit;
}

void UOldManMovementComponent::SetGravityByRaycast(bool bInstant)
{
    FVector HitLocation, HitNormal;
    if (PerformRaycast(HitLocation, HitNormal))
    {
        SetGravityFromSurfaceNormal(HitNormal, bInstant);
    }
    else
    {
        // 如果没有检测到表面，保持当前重力方向
        // 不重置为默认重力，因为可能是在空中
    }
}

void UOldManMovementComponent::SetGravityFromSurfaceNormal(FVector SurfaceNormal, bool bInstant)
{
    // 使用表面法线的反方向作为重力方向
    FVector NewGravityDirection = -SurfaceNormal;
    NewGravityDirection.Normalize();

    TargetGravityDirection = NewGravityDirection;
    bUseCustomGravity = true;

    if (bInstant)
    {
        CurrentGravityDirection = TargetGravityDirection;
        UpdateCharacterOrientation();
    }
}

void UOldManMovementComponent::ResetGravityDirection(bool bInstant)
{
    TargetGravityDirection = DefaultGravityDirection;

    if (bInstant)
    {
        CurrentGravityDirection = TargetGravityDirection;
        bUseCustomGravity = false;
        UpdateCharacterOrientation();
    }
}

void UOldManMovementComponent::UpdateCharacterOrientation()
{
    if (!CharacterOwner || !bUseCustomGravity)
        return;

    ACharacter* OwnerCharacter = GetCharacterOwner();
    if (!OwnerCharacter)
        return;

    // 在自定义重力下，让角色始终"站立"在当前的表面上
    FVector NewUp = -CurrentGravityDirection;

    // 获取当前的前方向
    FVector CurrentForward = OwnerCharacter->GetActorForwardVector();

    // 将前方向投影到新的"地面"平面上
    FVector NewForward = FVector::VectorPlaneProject(CurrentForward, NewUp).GetSafeNormal();

    // 如果投影后长度为0，使用默认前方向
    if (NewForward.IsNearlyZero())
    {
        // 尝试使用世界前方向
        NewForward = FVector::VectorPlaneProject(FVector(1, 0, 0), NewUp).GetSafeNormal();
        if (NewForward.IsNearlyZero())
        {
            // 如果还是零，使用世界右方向
            NewForward = FVector::VectorPlaneProject(FVector(0, 1, 0), NewUp).GetSafeNormal();
        }
    }

    // 计算右向量
    FVector NewRight = FVector::CrossProduct(NewUp, NewForward).GetSafeNormal();

    // 重新计算前向量以确保正交
    NewForward = FVector::CrossProduct(NewRight, NewUp).GetSafeNormal();

    // 构建旋转矩阵
    FRotator NewRotation = FRotationMatrix::MakeFromXZ(NewForward, NewUp).Rotator();

    // 应用新的旋转
    OwnerCharacter->SetActorRotation(NewRotation);
}