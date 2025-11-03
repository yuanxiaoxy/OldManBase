#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OldManMovementComponent.generated.h"

UCLASS()
class OLDMAN_API UOldManMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UOldManMovementComponent();

public:
    // 当前重力方向
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
    FVector CurrentGravityDirection;

    // 是否使用自定义重力
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
    bool bUseCustomGravity;

private:
    // 使用引擎内置函数设置重力方向
    void SetCustomGravityDirection(const FVector& NewGravityDirection);
};