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
	TArray<UMaterialInterface*> SafeMaterials;

	// 地板块静态网格体组件（根组件）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossGrid|组件")
	UStaticMeshComponent* GridMeshComp = nullptr;

	UPROPERTY(EditAnywhere, Category = "BossGrid")
	float FlashFrequency = 0.2f;

	int32 GridX;

	int32 GridY;

	bool bPlayerOnGrid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossGrid|状态", meta = (AllowPrivateAccess = "true"))
	EGridState CurrentGridState = EGridState::Safe;

	

private:
	// 当前地块状态

	FTimerHandle TimerHandle_FlashSwitch;  // 材质切换计时器
	FTimerHandle TimerHandle_FlashDuration; // 总时长计时器
	bool m_bIsFlashing = false;

	FTimerHandle m_TimerHandle_SwitchDanger;





public:	
	// Sets default values for this actor's properties
	ABossGrid();


	UFUNCTION(BlueprintCallable, Category = "BossGrid")
	void Initialize(int32 X, int32 Y, FVector generate, int32 delta);

	// 切换为危险区域块
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToDanger(float FlashTime);

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
	//蓝图用
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerInDanger(AActor* OtherActor);
	//蓝图用
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerInSafe(AActor* OtherActor);

	// 玩家离开地块事件
	UFUNCTION()
	void OnGridEndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//蓝图用
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerOutDanger(AActor* OtherActor);

	



private:
	void SetPos(int32 X, int32 Y, FVector generate, int32 delta);

	void OnSwitchToDangerDelayed(UMaterialInterface* nextMat);

	void ToggleFlashMaterial();




protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;




};


