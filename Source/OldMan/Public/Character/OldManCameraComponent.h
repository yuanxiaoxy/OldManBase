// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "OldManCameraComponent.generated.h"

/**
 * ���������� - �������ϵͳ
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
    // ========== �����ʼ�� ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void InitializeCameraComponents(USpringArmComponent* InCameraBoom, UCameraComponent* InFollowCamera);

    // ========== ������� ==========
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

    // ========== ���ģʽ ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetThirdPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFirstPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFreeLookMode();

    // ========== ���� ==========
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
    // ���ɱ��������
    UPROPERTY()
    USpringArmComponent* CameraBoom;

    // ��������������
    UPROPERTY()
    UCameraComponent* FollowCamera;

    // Ŀ���ɫ
    UPROPERTY()
    AActor* TargetActor;

    // ƽ�������ת����
    FRotator CurrentCameraRotation;
    FRotator DesiredCameraRotation;

    // ���봦�� - ��Ϊֱ�����ö����ۼ�
    float CurrentLookUpInput;
    float CurrentTurnInput;

    // ����ƽ������
    float SmoothedLookUpInput;
    float SmoothedTurnInput;

    // ����ƽ������
    UPROPERTY(EditAnywhere, Category = "Camera|Input")
    float InputSmoothingInterpSpeed = 12.0f;

    // ����ر���
    bool bIsShaking;
    float ShakeIntensity;
    float ShakeDuration;
    float ShakeElapsed;

    // ��ǰ���ģʽ
    FName CurrentCameraMode;

    // ÿ֡����
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ���º���
    void UpdateInputSmoothing(float DeltaTime);
    void UpdateCameraRotation(float DeltaTime);
    void UpdateCameraPosition(float DeltaTime);
};