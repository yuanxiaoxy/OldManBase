// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "OldManCameraComponent.generated.h"

/**
 * 老人相机组件 - 相机控制系统
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OLDMAN_API UOldManCameraComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOldManCameraComponent();

protected:
    virtual void BeginPlay() override;

public:
    // ========== 相机初始化 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera);

    // ========== 相机控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraTarget(AActor* TargetActor);

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

    // ========== 属性 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraLagSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraRotationLagSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraPitchMin = -70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraPitchMax = 70.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    bool bUseCameraSmoothing = true;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraRotationInterpSpeed = 10.0f;

private:
    // 弹簧臂组件引用
    UPROPERTY()
    USpringArmComponent* CameraBoom;

    // 跟随相机组件引用
    UPROPERTY()
    UCameraComponent* FollowCamera;

    // 目标角色
    UPROPERTY()
    AActor* TargetActor;

    // 平滑相机旋转变量
    FRotator CurrentCameraRotation;
    FRotator DesiredCameraRotation;

    // 输入处理 - 改为直接设置而非累加
    float CurrentLookUpInput;
    float CurrentTurnInput;

    // 输入平滑处理
    float SmoothedLookUpInput;
    float SmoothedTurnInput;

    // 输入平滑参数
    UPROPERTY(EditAnywhere, Category = "Camera|Input")
    float InputSmoothingInterpSpeed = 12.0f;

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
};