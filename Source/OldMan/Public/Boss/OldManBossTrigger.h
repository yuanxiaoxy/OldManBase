// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OldManBossTrigger.generated.h"

class AOldManHead;

UCLASS()
class OLDMAN_API AOldManBossTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOldManBossTrigger();

	// 碰撞组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossTrigger")
	class UBoxComponent* CollisionComponent;

	// 视觉组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossTrigger")
	class UStaticMeshComponent* MeshComponent;

	// OldManHead 引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossTrigger")
	AOldManHead* OldManHeadRef;

	// 击中后增加的进度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossTrigger")
	float ProgressToAdd;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// 碰撞检测回调
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 处理伤害事件
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

};
