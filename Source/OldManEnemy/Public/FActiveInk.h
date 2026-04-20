#pragma once
#include "CoreMinimal.h"
#include "FActiveInk.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FActiveInk {
    GENERATED_BODY()

    UTexture2D* Texture; // 墨渍贴图

    FVector2D NormalizedPosition;  // 屏幕位置

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk")
    float Duration;

    float Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk")
    float BaseRotationDeg = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk")
    float SwingAngleDeg = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk", meta = (ClampMin = "0.0"))
    float SwingBackDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk", meta = (ClampMin = "0.0"))
    float MultiSwingBackDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk", meta = (ClampMin = "0.0"))
    float SwingToggleInterval = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk", meta = (ClampMin = "0.0"))
    float SpawnDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk", meta = (ClampMin = "0.0"))
    float PopDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk",
        meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
    float NormalizedWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ActikveInk",
        meta = (UIMin = "0.0", UIMax = "1.0", ClampMin = "0.0", ClampMax = "1.0"))
    float NormalizedHeight;
};