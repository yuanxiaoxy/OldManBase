// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SingletonBase/ActorSingletonBase.h"

#include "BossGridManager.generated.h"
class ABossGrid;
class AActor;


/// <summary>
/// 管理地块状态切换和函数调用
/// </summary>

UCLASS()
class OLDMANBOSSGRID_API ABossGridManager : public AActorSingletonBase
{
	GENERATED_BODY()

	DECLARE_ACTOR_SINGLETON(ABossGridManager);

//	属性
public:

	UPROPERTY(EditAnywhere, Category = "BossMapSet", meta = (UIMin = 0, UIMax = 10, ClampMin = 0, ClampMax = 10))
	AActor* GenerateCenter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 MaxJumpGridCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float FlashDuriation = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	TSubclassOf<ABossGrid> BossGridClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GenerationDelta = 10;

	UPROPERTY(EditAnywhere, Category = "BossMapSet", meta = (UIMin = 1, UIMax = 20, ClampMin = 1, ClampMax = 20))
	int32 MapWidth = 7;

	UPROPERTY(EditAnywhere, Category = "BossMapSet", meta = (UIMin = 1, UIMax = 20, ClampMin = 1, ClampMax = 20))
	int32 MapHeight = 7;

	UPROPERTY(EditAnywhere, Category = "BossMapSet", meta = (UIMin = 0, UIMax = 10, ClampMin = 0, ClampMax = 10))
	float DangerProbability = 8;

	
	

private:
	bool m_bHasSafeInRange = false;
	TArray<ABossGrid*> m_SafeGrids;
	ACharacter* m_Player = nullptr;
	// 动态二维网格（根据宽高自动创建）
	TArray<TArray<ABossGrid*>> m_Grids;

	FVector m_GenerateVector;
	
	FVector m_GridExtent;
	bool m_bSetExtent = false;



//	方法
public:	
	// Sets default values for this actor's properties
	ABossGridManager();

	UFUNCTION(BlueprintCallable, Category = "BossGridManager")
	void GenerateMap();

	/// <summary>
	/// 随机设置地块状态，保证玩家在跳跃范围内至少有一个安全格子
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "BossGridManager")
	void RandomSetMap();

	UFUNCTION(BlueprintCallable, Category = "BossGridManager")
	void RandomSetMapWithPoints(
		FIntPoint start1,
		FIntPoint start2,
		FIntPoint start3,
		FIntPoint start4,
		FIntPoint end
		);
		
	/// <summary>
	/// 设置所有格子为安全状态（用于玩家死亡后重置地图）
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "BossGridManager")
	void AllSetSafe();

	UFUNCTION(BlueprintCallable, Category = "BossGridManager")
	FIntPoint GetPlayerGridIndex();

	

private:

	bool IsInJumpRange(int32 PlayerX, int32 PlayerY, int32 TargetX, int32 TargetY);


	void ForceRandomSafeInJumpRange(FIntPoint PlayerGrid);








protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;



private:
	float TimerInterval = 0.0f;
	const float WaitTime = 4.0f; // 4秒触发一次


};
