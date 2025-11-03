// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManBulletBase.h"
#include "OldManBullet.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class OLDMANITEM_API AOldManBullet : public AOldManBulletBase
{
	GENERATED_BODY()
	
protected:
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParam")
	float HomingStrength = 5.0f; // 转向强度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BulletParam")
	float HomingStartDistance = 500.0f; // 开始转向的距离

public:
	virtual void InitializeBullet(const FVector& StartDirection, AActor* NewTarget = nullptr) override;

private:
	UPROPERTY()
	AActor* TargetActor;

	UPROPERTY()
	bool bHasTarget;
};
