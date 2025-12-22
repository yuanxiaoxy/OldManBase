#pragma once
#include "FActiveInk.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FActiveInk {
    GENERATED_BODY()
    UTexture2D* Texture; // 墨渍贴图
    FVector2D NormalizedPosition;  // 屏幕位置
    float Duration;      // 总持续时间
    float Age;           // 已存在时间
    float NormalizedWidth;
    float NormalizedHeight;
};