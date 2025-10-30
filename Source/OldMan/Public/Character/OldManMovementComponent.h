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

    // 当前站立表面的法线
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gravity")
    FVector CurrentSurfaceNormal;

protected:
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 重写物理更新函数
    virtual void PhysCustom(float deltaTime, int32 Iterations) override;

    // 重写移动函数以检测表面法线
    virtual void MoveAlongFloor(const FVector& InVelocity, float DeltaSeconds, FStepDownResult* OutStepDown = nullptr) override;

public:
    // 使用不同的函数名避免与父类冲突
    void SetCustomGravityDirection(FVector NewGravityDirection, bool bInstant = false);

    // 基于表面法线设置重力
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetGravityFromSurfaceNormal(FVector SurfaceNormal, bool bInstant = false);

    // 重置重力方向
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void ResetCustomGravityDirection(bool bInstant = false);

    // 应用自定义重力
    void ApplyCustomGravity(float DeltaTime);

    // 更新角色朝向
    void UpdateCharacterOrientation();

    // 获取站立表面的法线
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    FVector GetSurfaceNormal() const { return CurrentSurfaceNormal; }

private:
    FVector TargetGravityDirection;

    // 表面检测
    void UpdateSurfaceNormal();
    FVector FindSurfaceNormal() const;
};