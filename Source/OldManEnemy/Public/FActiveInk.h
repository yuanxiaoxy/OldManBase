#pragma once
#include "FActiveInk.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FActiveInk {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk")
    UTexture2D* Texture; // 墨渍贴图


    FVector2D NormalizedPosition;  // 屏幕位置

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk")
    float Duration;      // 总持续时间

    float Age;           // 已存在时间

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk",
        meta = (UIMin = 0.0, UIMax = 1.0, ClampMin = 0.0, ClampMax = 1.0))
    float NormalizedWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk",
        meta = (UIMin = 0.0, UIMax = 1.0, ClampMin = 0.0, ClampMax = 1.0))
    float NormalizedHeight;
};