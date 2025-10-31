// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManItemBase.h"
#include "OldManBulletBase.generated.h"

USTRUCT(BlueprintType)
struct FBulletBaseParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletBaseParam")
	float InitialSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletBaseParam")
	float MaxSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletBaseParam")
	float LifeSpan = 5.0f; // 子弹生命周期
};

UCLASS(Blueprintable)
class OLDMANITEM_API AOldManBulletBase : public AOldManItemBase
{
	GENERATED_BODY()
	
public:
	AOldManBulletBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UProjectileMovementComponent* ProjectileMovement;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BulletParam")
	FBulletBaseParam bulletBaseParam;

public:
	// 初始化子弹
	UFUNCTION(BlueprintCallable, Category = "InitializeBullet")
	virtual void InitializeBullet(const FVector& direction, AActor* targetActor = nullptr);

	UFUNCTION(BlueprintImplementableEvent, Category = "OnHitAttack")
	void Attacked();

	// 碰撞开始事件
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);
};
