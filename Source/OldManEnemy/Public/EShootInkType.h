#pragma once

#include "CoreMinimal.h"
#include "EShootInkType.generated.h"

UENUM(BlueprintType)
enum class EShootInkType : uint8
{
	Single	UMETA(DisplayName = "单弹窗"),
	Multi	UMETA(DisplayName = "多弹窗")
};