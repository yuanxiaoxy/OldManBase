#pragma once

#include "CoreMinimal.h"
#include "EShootInkType.generated.h"

UENUM(BlueprintType)
enum class EApproachEnemyInkMode : uint8
{
    Single UMETA(DisplayName = "Single"),
    Multi UMETA(DisplayName = "Multi")
};