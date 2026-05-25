// File: OldManCameraAnimationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OldManCameraAnimationAsset.h"
#include "Engine/World.h"
#include "OldManCameraAnimationComponent.generated.h"

class UOldManCameraComponent;
class AOldManCharacter;
class ULevelSequencePlayer;
class ULevelSequence;
class ACameraActor;
class AOldManCameraAnimationTrigger;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraAnimationStarted, const FOldManCameraAnimationData&, AnimationData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraAnimationFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraAnimationPaused);

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

	// ========== 公共接口 ==========
	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void InitializeCameraAnimation(UOldManCameraComponent* InCameraComponent, AOldManCharacter* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void StartCameraAnimation(const FOldManCameraAnimationData& AnimationData, bool bForceRestart = false);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetAnimationTarget(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void StopCameraAnimation(bool bImmediate = false);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void PauseCameraAnimation();

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	bool IsCameraAnimationPlaying() const { return bIsAnimationPlaying; }

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetRuntimeFollowTarget(AActor* FollowTarget);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	FOldManCameraAnimationData GetCurrentAnimationData() const { return CurrentAnimationData; }

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetStopTrigger(AOldManCameraAnimationTrigger* StopTrigger) { CurrentStopTrigger = StopTrigger; }

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	bool IsStopTriggerValid(AOldManCameraAnimationTrigger* StopTrigger) const;

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetOrbitParameters(float Radius, float Height, float Speed, bool bClockwise);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetMoveToParameters(const FVector& PositionOffset, ECameraOffsetSpace OffsetSpace = ECameraOffsetSpace::World);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetFollowPlayerParameters(const FVector& CameraOffset, ECameraOffsetSpace OffsetSpace = ECameraOffsetSpace::World, bool bWithRotation = true);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetFollowPlayerAndLookAtParameters(float SphereRadius, float SphereHeight, const FVector& CameraOffset, const FRotator& CameraRotationOffset, bool bUseExtensionLineOffset = true);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	FVector GetAnimationCameraLocation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	FRotator GetAnimationCameraRotation() const;

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetMovementLerpParameters(bool bUsePositionLerp, float PositionLerpSpeed = 5.0f, bool bUseRotationLerp = true, float RotationLerpSpeed = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SetExposeMousePosition(bool bExposeMouse);

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	float GetAnimationBlendAlpha() const { return AnimationBlendAlpha; }

	UFUNCTION(BlueprintCallable, Category = "Camera Animation")
	void SwitchCameraAnimation(const FOldManCameraAnimationData& NewAnimationData, float TransitionTime = 0.5f);

	// 事件委托
	UPROPERTY(BlueprintAssignable, Category = "Camera Animation")
	FOnCameraAnimationStarted OnCameraAnimationStarted;

	UPROPERTY(BlueprintAssignable, Category = "Camera Animation")
	FOnCameraAnimationFinished OnCameraAnimationFinished;

	UPROPERTY(BlueprintAssignable, Category = "Camera Animation")
	FOnCameraAnimationPaused OnCameraAnimationPaused;

private:
	// ========== 状态变量 ==========
	UPROPERTY()
	bool bIsAnimationPlaying = false;

	UPROPERTY()
	bool bIsPaused = false;

	UPROPERTY()
	bool bIsBlendingIn = false;

	UPROPERTY()
	bool bIsBlendingOut = false;

	UPROPERTY()
	bool bIsSwitchingAnimation = false;

	UPROPERTY()
	float SwitchTransitionTime = 0.0f;

	UPROPERTY()
	float SwitchTransitionElapsed = 0.0f;

	UPROPERTY()
	float AnimationBlendAlpha = 0.0f;

	UPROPERTY()
	float AnimationInternalBlendAlpha = 0.0f;

	UPROPERTY()
	float AnimationElapsedTime = 0.0f;

	UPROPERTY()
	float ExitBlendTime = 0.0f;

	UPROPERTY()
	float ExitBlendElapsed = 0.0f;

	// [FIX] 退出混合时保存的起始位置/旋转/FOV（固定不变）
	UPROPERTY()
	FVector ExitStartLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator ExitStartRotation = FRotator::ZeroRotator;

	UPROPERTY()
	float ExitStartFOV = 90.0f;

	UPROPERTY()
	float AnimationInternalLerpAlpha = 0.0f;

	UPROPERTY()
	FVector PreviousAnimationEndLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator PreviousAnimationEndRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector SwitchStartLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator SwitchStartRotation = FRotator::ZeroRotator;

	UPROPERTY()
	FOldManCameraAnimationData SwitchTargetData;

	UPROPERTY()
	FVector LerpCameraLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator LerpCameraRotation = FRotator::ZeroRotator;

	UPROPERTY()
	bool bFirstFrameAfterSwitch = false;

	UPROPERTY()
	FVector PostSwitchTargetLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator PostSwitchTargetRotation = FRotator::ZeroRotator;

	// ========== 组件引用 ==========
	UPROPERTY()
	UOldManCameraComponent* CameraComponent = nullptr;

	UPROPERTY()
	AOldManCharacter* Character = nullptr;

	UPROPERTY()
	ULevelSequencePlayer* SequencePlayer = nullptr;

	UPROPERTY()
	ACameraActor* CameraAnimationActor = nullptr;

	// ========== 动画数据 ==========
	UPROPERTY()
	FOldManCameraAnimationData CurrentAnimationData;

	UPROPERTY()
	FOldManCameraAnimationData PreviousAnimationData;

	UPROPERTY()
	AOldManCameraAnimationTrigger* CurrentStopTrigger = nullptr;

	// ========== 原始状态备份 ==========
	UPROPERTY()
	AActor* OriginalViewTarget = nullptr;

	UPROPERTY()
	bool bOriginalPlayerInputState = true;

	UPROPERTY()
	bool bOriginalPlayerVisibility = true;

	UPROPERTY()
	float OriginalFOV = 90.0f;

	UPROPERTY()
	FVector OriginalCameraRelativeOffset = FVector::ZeroVector;

	UPROPERTY()
	FRotator OriginalCameraRelativeRotation = FRotator::ZeroRotator;

	// ========== 轨道动画变量 ==========
	UPROPERTY()
	float CurrentOrbitAngle = 0.0f;

	UPROPERTY()
	float CurrentOrbitRadius = 500.0f;

	UPROPERTY()
	float CurrentOrbitHeight = 100.0f;

	UPROPERTY()
	float CurrentOrbitSpeed = 45.0f;

	UPROPERTY()
	bool bCurrentClockwiseOrbit = true;

	// ========== 私有方法 ==========
	void UpdateCameraAnimation(float DeltaTime);
	void UpdateBlend(float DeltaTime);
	void UpdateExitBlend(float DeltaTime);
	void UpdateSwitchTransition(float DeltaTime);
	void UpdateOrbit(float DeltaTime);

	FVector CalculateAnimatedCameraLocation() const;
	FRotator CalculateAnimatedCameraRotation() const;
	FVector CalculateOrbitLocation() const;

	FVector ApplyPositionLerp(float DeltaTime, const FVector& TargetLocation);
	FRotator ApplyRotationLerp(float DeltaTime, const FRotator& TargetRotation);

	void CreateCameraAnimationActor();
	void DestroyCameraAnimationActor();
	void SwitchToAnimationCamera();
	void SwitchBackToPlayerCamera();

	void DisablePlayerInput();
	void EnablePlayerInput();
	void ApplyFOV(float InFOV);

	void StartLevelSequence();
	void StopLevelSequence();

	UFUNCTION()
	void OnSequenceFinished();

	void UpdateAlignToTargetAnimation();
	void UpdateLookAtObjectAnimation();
	void UpdateOrbitAnimation();
	void UpdateMoveToTargetAndLookAtPlayerAnimation();
	void UpdateFollowPlayerAnimation();
	void UpdateFollowPlayerAndLookAtTargetAnimation();

	bool CheckStopConditions(float DeltaTime);

	void StartExitBlend();

	FVector CalculateTargetCameraLocation() const;
	FRotator CalculateTargetCameraRotation() const;

	void ApplyAnimationInternalLerpToCamera(float DeltaTime, const FVector& TargetLocation, const FRotator& TargetRotation);

	void HandleMouseExposure(bool bExpose);

	void SaveCurrentAnimationState();

	void StartSwitchAnimation(const FOldManCameraAnimationData& NewAnimationData, float TransitionTime);

	void HandleFirstFrameAfterSwitch(float DeltaTime);
};