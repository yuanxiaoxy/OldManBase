// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManItemBase.h"
#include "FloatingItem.generated.h"

UENUM(BlueprintType)
enum class EFloatingItemState : uint8
{
	Idle,
	SinkRiseCycle
};

UCLASS(Blueprintable)
class OLDMANITEM_API AFloatingItem : public AOldManItemBase
{
	GENERATED_BODY()

public:
	AFloatingItem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 垂直漂浮
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Vertical")
	float VerticalAmplitude = 15.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Vertical")
	float VerticalSpeed1 = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Vertical")
	float VerticalSpeed2 = 3.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Vertical")
	float VerticalSpeed3 = 5.8f;

	// 水平漂浮
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Horizontal")
	float HorizontalAmplitude = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Horizontal")
	float HorizontalSpeedX = 0.9f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Horizontal")
	float HorizontalSpeedY = 1.1f;

	// 旋转摆动
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Rotation")
	float RotateAmplitude = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Rotation")
	float RotateSpeedPitch = 1.3f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Rotation")
	float RotateSpeedRoll = 1.8f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Float Movement|Rotation")
	float RotateSpeedYaw = 0.5f;

	// 下沉/上浮
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sink & Rise")
	float SinkDepth = 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sink & Rise")
	float TotalCycleDuration = 1.6f;

protected:
	FVector InitialLocation;
	FRotator InitialRotation;
	EFloatingItemState CurrentState = EFloatingItemState::Idle;
	float RunningTime = 0.0f;
	float CycleProgress = 0.0f;

	UPROPERTY()
	AActor* AttachedPlayer = nullptr;

	UFUNCTION()
	void OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	void StartSinkRiseCycle(AActor* PlayerActor);
	void EndSinkRiseCycle();

	FVector CalculateFloatOffset();
	FRotator CalculateFloatRotation();
};