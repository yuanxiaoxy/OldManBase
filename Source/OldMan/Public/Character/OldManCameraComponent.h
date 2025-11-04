// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "OldManCharacterAttributes.h"
#include "OldManCameraComponent.generated.h"

class AOldManCharacter;
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OLDMAN_API UOldManCameraComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOldManCameraComponent();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    float CurCameraDistance = 0.0f;

public:
    // ========== 相机初始化 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera, FOldManCameraData CameraData);

    // ========== 相机控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraTarget(AOldManCharacter* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraOffset(const FVector& Offset);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraInput(float rawLookUpInput, float rawTurnInput);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    FRotator GetCameraRotation();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ShakeCamera(float Intensity, float Duration);

    // ========== 相机模式 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetThirdPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFirstPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFreeLookMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void GetActorsInCone(
        FOldManDetectionData DetectionData,
        const FName ValidTag,
        TArray<AActor*>& OutActors,
        TArray<float>& OutDistances,
        TArray<float>& OutAngles
    );


    UFUNCTION(BlueprintCallable, Category = "Camera")
    void DrawConeVisualization(
        UWorld* World,
        const FVector& Origin,
        const FVector& Direction,
        float ConeLength,
        float ConeAngle,
        FColor Color,
        float Duration
    );

private:
    // 弹簧臂组件引用
    UPROPERTY()
    USpringArmComponent* CameraBoom;

    // 跟随相机组件引用
    UPROPERTY()
    UCameraComponent* FollowCamera;

    UPROPERTY()
    AOldManCharacter* CachedOldManCharacter;

    UPROPERTY()
    FOldManCameraData MyCameraData;

    UPROPERTY()
    FOldManDetectionData MyDetectionData;

    // 平滑相机旋转变量
    FRotator CurrentCameraRotation;
    FRotator DesiredCameraRotation;

    // 输入处理 - 改为直接设置而非累加
    float CurrentLookUpInput;
    float CurrentTurnInput;

    // 输入平滑处理
    float SmoothedLookUpInput;
    float SmoothedTurnInput;

    bool bHasRecentInput;

    // 重力对齐标记
    bool bNeedsGravityAlignment;

    // 输入平滑参数
    UPROPERTY(EditAnywhere, Category = "Camera|Input")
    float InputSmoothingInterpSpeed = 12.0f;

    // 重力方向处理
    UPROPERTY()
    FVector CurrentGravityDirection;

    UPROPERTY()
    FVector DesiredGravityDirection;

    // 重力方向平滑参数
    UPROPERTY(EditAnywhere, Category = "Camera|Gravity")
    float GravityRotationInterpSpeed = 8.0f;


    // 震动相关变量
    bool bIsShaking;
    float ShakeIntensity;
    float ShakeDuration;
    float ShakeElapsed;

    // 当前相机模式
    FName CurrentCameraMode;

    // 每帧更新
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 更新函数
    void UpdateInputSmoothing(float DeltaTime);
    void UpdateCameraRotation(float DeltaTime);
    void UpdateCameraPosition(float DeltaTime);
    void UpdateGravityAlignment(float DeltaTime); // 新增：重力对齐

    // 新增：重力方向辅助函数
    FRotator GetRotationFromGravityDirection(const FVector& GravityDir) const;
    FRotator MakeRotationFromGravity(const FVector& Forward, const FVector& GravityDir) const;
};