// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boss/OldManHead.h"
#include "OldManBeamAttack.generated.h"

UCLASS()
class OLDMAN_API AOldManBeamAttack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOldManBeamAttack();

	/** 目标老人头 */
	UPROPERTY(EditAnywhere, Category = "Halo")
	AOldManHead* TargetHead;

	/** 移动速度（单位/秒） */
	UPROPERTY(EditAnywhere, Category = "Halo")
	float MoveSpeed = 10;

	/** 碰撞组件，用作根组件及触发检测 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Halo")
	class USphereComponent* CollisionComponent;

	/** 是否已到达目标（防止重复触发） */
	UPROPERTY(EditAnywhere, Category = "Halo")
	bool Reached = false;

	/** 成功命中时增加的进度值 */
	UPROPERTY(EditAnywhere, Category = "Halo")
	float AddProgress = 1;

	/** 失败时减少的进度值（当前未使用） */
	UPROPERTY(EditAnywhere, Category = "Halo")
	float DeleteProgress = 0;

	/** 持续时间 */
	UPROPERTY(EditAnywhere, Category = "Halo")
	float ContinueTime = 0;
protected:
	virtual void BeginPlay() override;

	/** 重叠事件处理：检测是否与老人头重叠 */
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	/** 死亡方法 */
	void DestoryMe();

public:
	virtual void Tick(float DeltaTime) override;

};
