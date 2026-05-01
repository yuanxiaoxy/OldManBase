#pragma once
#include "CoreMinimal.h"
#include "EBossGridState.generated.h"


// 地块状态枚举
UENUM(BlueprintType)
enum class EGridState : uint8
{
	Safe,       // 安全
	Danger,     // 危险
	Flashing    // 闪烁中
};