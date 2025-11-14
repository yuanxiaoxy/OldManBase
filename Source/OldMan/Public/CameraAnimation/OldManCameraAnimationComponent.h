// OldManCameraAnimationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraAnimation/OldManCameraAnimationAsset.h"
#include "OldManCameraAnimationComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraAnimationStarted, const FOldManCameraAnimationData&, AnimationData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraAnimationFinished);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OLDMAN_API UOldManCameraAnimationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOldManCameraAnimationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 初始化 - 移动到public区域
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void InitializeCameraAnimation(class UOldManCameraComponent* InCameraComponent, class AOldManCharacter* InCharacter);

    // 相机动画控制
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void StartCameraAnimation(const FOldManCameraAnimationData& AnimationData);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void StopCameraAnimation();

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    bool IsCameraAnimationPlaying() const { return bIsAnimationPlaying; }

    // 委托
    UPROPERTY(BlueprintAssignable, Category = "Camera Animation")
    FOnCameraAnimationStarted OnCameraAnimationStarted;

    UPROPERTY(BlueprintAssignable, Category = "Camera Animation")
    FOnCameraAnimationFinished OnCameraAnimationFinished;

private:
    // 动画状态
    UPROPERTY()
    bool bIsAnimationPlaying = false;

    UPROPERTY()
    FOldManCameraAnimationData CurrentAnimationData;

    UPROPERTY()
    float AnimationBlendAlpha = 0.0f;

    UPROPERTY()
    bool bIsBlendingIn = false;

    UPROPERTY()
    bool bIsBlendingOut = false;

    // 原始相机数据（用于混合和恢复）
    UPROPERTY()
    FVector OriginalCameraLocation;

    UPROPERTY()
    FRotator OriginalCameraRotation;

    UPROPERTY()
    float OriginalCameraDistance;

    UPROPERTY()
    float OriginalCameraFOV;

    // 引用
    UPROPERTY()
    class UOldManCameraComponent* CameraComponent;

    UPROPERTY()
    class AOldManCharacter* Character;

    // 动画更新函数
    void UpdateCameraAnimation(float DeltaTime);
    void UpdateBlend(float DeltaTime);
    void UpdateLookAtObject(float DeltaTime);
    void UpdateLookAtDirection(float DeltaTime);
    void UpdateOrbitMovement(float DeltaTime);
    void UpdatePathMovement(float DeltaTime);

    // 辅助函数
    FVector CalculateAnimatedCameraLocation();
    FRotator CalculateAnimatedCameraRotation();
    void ApplyCameraTransform(const FVector& Location, const FRotator& Rotation, float Alpha);
};