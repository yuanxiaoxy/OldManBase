// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EBossGridState.h"
#include "BossGrid.generated.h"

UCLASS()
class OLDMANBOSSGRID_API ABossGrid : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossGrid")
	TArray<UMaterialInterface*> DangerMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BossGrid")
	UMaterialInterface* SafeMaterial;

	// 地板块静态网格体组件（根组件）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BossGrid|组件")
	UStaticMeshComponent* GridMeshComp;

	UPROPERTY(EditAnywhere, Category = "BossGrid")
	float FlashFrequency = 0.2f;

	int32 GridX;

	int32 GridY;

	bool bPlayerOnGrid = false;

	

private:
	// 当前地块状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossGrid|状态", meta = (AllowPrivateAccess = "true"))
	EGridState CurrentGridState = EGridState::Safe;

	float FlashSwitchTimer;
	float FlashDurationTimer;



public:	
	// Sets default values for this actor's properties
	ABossGrid();


	UFUNCTION(BlueprintCallable, Category = "BossGrid")
	void Initialize(int32 X, int32 Y, FVector generate);

	// 切换为危险区域块
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToDanger();

	// 切换为安全区域块
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToSafe();

	// 地板块闪烁（传入闪烁时长，单位秒）
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToFlash(float FlashTime);

	// 玩家踩入地块事件
	UFUNCTION()
	void OnGridBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// 玩家离开地块事件
	UFUNCTION()
	void OnGridEndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	



private:
	void SetPos(int32 X, int32 Y, FVector generate);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;




};


