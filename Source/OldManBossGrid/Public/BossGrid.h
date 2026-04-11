// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

	// 闪烁持续时间（秒）
	UPROPERTY(EditAnywhere, Category = "BossGrid")
	float FlashDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category = "BossGrid")
	float FlashFrequency = 0.2f;

	
public:	
	// Sets default values for this actor's properties
	ABossGrid();


	UFUNCTION(BlueprintCallable, Category = "BossGrid")
	void Initialize();

	// 切换为危险区域块
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToDanger();

	// 切换为安全区域块
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToSafe();

	// 地板块闪烁（传入闪烁时长，单位秒）
	UFUNCTION(BlueprintCallable, Category = "BossGrid|行为")
	void SwitchToFlash(int32 FlashTime);

	// 玩家踩入地块事件
	UFUNCTION()
	void OnGridBeginOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// 玩家离开地块事件
	UFUNCTION()
	void OnGridEndOverlap(UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;




private:
	// 当前地块状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BossGrid|状态", meta = (AllowPrivateAccess = "true"))
	EGridState CurrentGridState = EGridState::Safe;

	// 闪烁计时器
	float FlashTimer;

	float FlashSwitchTimer;

};


// 地块状态枚举
UENUM(BlueprintType)
enum class EGridState : uint8
{
	Safe,       // 安全
	Danger,     // 危险
	Flashing    // 闪烁中
};