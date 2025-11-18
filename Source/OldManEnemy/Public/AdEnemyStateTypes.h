// AdEnemyStateTypes.h
#pragma once

#include "CoreMinimal.h"

#include "AdEnemyStateTypes.generated.h" 

UENUM(BlueprintType)
enum class EAdMonsterState : uint8
{
    Patrol         UMETA(DisplayName = "巡逻"),
    Tracking       UMETA(DisplayName = "追踪"),
    AttackPreparation UMETA(DisplayName = "攻击准备"),
    Attacking      UMETA(DisplayName = "攻击"),
    Hurt           UMETA(DisplayName = "受伤"),
    Dead           UMETA(DisplayName = "死亡")
};