#include "Character/OldManMovementComponent.h"
#include "GameFramework/Character.h"

UOldManMovementComponent::UOldManMovementComponent()
{
    // 启用行走模式支持各种角度
    SetWalkableFloorZ(0.1f);

    // 禁用自动旋转，由角色类控制
    bOrientRotationToMovement = false;
}
