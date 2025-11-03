// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet/OldManBullet.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/Engine.h"


void AOldManBullet::InitializeBullet(const FVector& direction, AActor* targetActor)
{
    // 设置初始速度方向
    ProjectileMovement->Velocity = direction.GetSafeNormal() * bulletBaseParam.InitialSpeed;

    // 设置目标
    if (targetActor)
    {
        TargetActor = targetActor;
        bHasTarget = true;

        // 使用内置的制导功能
        ProjectileMovement->bIsHomingProjectile = true;
        ProjectileMovement->HomingTargetComponent = targetActor->GetRootComponent();
        ProjectileMovement->HomingAccelerationMagnitude = HomingStrength * 100.0f; // 调整系数
    }
}

void AOldManBullet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}