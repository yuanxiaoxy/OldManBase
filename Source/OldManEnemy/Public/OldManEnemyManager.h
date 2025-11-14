// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "Tickable.h"
#include "AdEnemyCharacter.h"
#include "GameFramework/Actor.h"
#include "SingletonBase/SingletonBase.h"
#include "EnemyPatrolPoint.h"
#include "FEnemyLocationInfo.h"
#include "OldManEnemyManager.generated.h"

class  AEnemyPatrolPoint;
class  AAdEnemyAIController;
class  ACharacter;
class  UEnemyObjectPool;




UCLASS(Blueprintable, BlueprintType)
class OLDMANENEMY_API UOldManEnemyManager : public USingletonBase, public FTickableGameObject
{
	GENERATED_BODY()
	
	DECLARE_SINGLETON(UOldManEnemyManager)
public:	
    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
	virtual void InitializeSingleton() override;
	
	UFUNCTION(BlueprintCallable, Category = "EnemyManager")
	void NotifyMonstersTracking();


    // FTickableGameObject 接口实现
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;
    virtual bool IsTickableWhenPaused() const override { return false; }
    virtual bool IsTickableInEditor() const override { return true; }


    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EnemyManager")
    static UOldManEnemyManager* GetEnemyManager() { return GetInstance(); }

    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void AddInfos(FEnemyLocationInfo info);

    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void GenerateEnemy();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void SetSpawnActive(FEnemyLocationInfo& infoRef, bool active);




    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    TSubclassOf<AAdEnemyCharacter> EnemyBlueprintClass;  // 用于在编辑器中指定要生成的蓝图Character类[10](@ref)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    int32 ObjectPoolCount = 25;  


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
    float SpawnInterval = 5.0f;  // 生成间隔，默认为5秒

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "EnemyManager")
    FTimerHandle EnemySpawnTimerHandle;  // 定时器句柄

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager")
	TArray<AAdEnemyAIController*> Enemys;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEnemyLocationInfo> EnemyInfos;

    

private:
    UObjectPoolManager* PoolManager;

	UOldManEnemyManager();

    //virtual UWorld* GetWorld() const override;

    bool _hasInitialze = false;

};

