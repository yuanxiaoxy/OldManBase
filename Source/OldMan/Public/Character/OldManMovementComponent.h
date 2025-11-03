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

protected:
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void PhysicsRotation(float DeltaTime) override;

public:
    // 当前重力方向
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
    FVector CurrentGravityDirection;

    // 默认重力方向
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    FVector DefaultGravityDirection;

    // 重力强度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float GravityStrength;

    // 是否使用自定义重力
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
    bool bUseCustomGravity;

    // 平滑过渡速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
    float GravityTransitionSpeed;

    // 射线检测距离
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
    float RaycastDistance;

    // 射线检测通道
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raycast")
    TEnumAsByte<ECollisionChannel> RaycastChannel;

    // 是否启用调试绘制
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugDrawing;

    // 使用射线检测设置重力方向
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetGravityByRaycast(bool bInstant = false);

    // 基于表面法线设置重力
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetGravityFromSurfaceNormal(FVector SurfaceNormal, bool bInstant = false);

    // 重置重力方向
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void ResetGravityDirection(bool bInstant = false);

    // 更新角色朝向
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void UpdateCharacterOrientation();

    // 执行射线检测
    UFUNCTION(BlueprintCallable, Category = "Raycast")
    bool PerformRaycast(FVector& OutHitLocation, FVector& OutHitNormal);

private:
    FVector TargetGravityDirection;

    // 射线检测结果
    FVector LastHitLocation;
    FVector LastHitNormal;
    bool bLastRaycastHit;

    // 应用自定义重力
    void ApplyCustomGravity(float DeltaTime);
};