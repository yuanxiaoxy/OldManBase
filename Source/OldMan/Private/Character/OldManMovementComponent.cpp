#include "Character/OldManMovementComponent.h"
#include "GameFramework/Character.h"

UOldManMovementComponent::UOldManMovementComponent()
{
    CurrentGravityDirection = FVector(0, 0, -1);
    bUseCustomGravity = false;

    // 启用行走模式支持各种角度
    SetWalkableFloorZ(0.1f);

    // 禁用自动旋转，由角色类控制
    bOrientRotationToMovement = false;
}

void UOldManMovementComponent::SetCustomGravityDirection(const FVector& NewGravityDirection)
{
    CurrentGravityDirection = NewGravityDirection;
    CurrentGravityDirection.Normalize();

    // 使用引擎内置函数设置重力方向
    SetGravityDirection(CurrentGravityDirection);
}