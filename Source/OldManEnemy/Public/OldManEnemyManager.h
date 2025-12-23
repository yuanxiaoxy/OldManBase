// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FActiveInk.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "AdEnemyCharacter.h"
#include "GameFramework/Actor.h"
#include "SingletonBase/SingletonBase.h"
#include "EnemyPatrolPoint.h"
#include "FEnemyLocationInfo.h"
#include "EditorUtilityActor.h"
#include "Containers/Map.h"
#include "OldManEnemyManager.generated.h"

class  AEnemyPatrolPoint;
class  AAdEnemyAIController;
class  ACharacter;
class  UEnemyObjectPool;
class  AApproachEnemyCharacter;
class  AOldManHUD;



UCLASS(Blueprintable, BlueprintType)
class OLDMANENEMY_API UOldManEnemyManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UOldManEnemyManager)
public:
    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    virtual void InitializeSingleton() override;

    

    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void NotifyMonstersTracking();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EnemyManager")
    static UOldManEnemyManager* GetEnemyManager() { return GetInstance(); }

#pragma region ApproachEnemy

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void StartApproachEnemyGenerator();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void StopApproachEnemyGenerator();

    // 更新屏幕敌人的属性，值为-1就保持原来的值不修改
    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void UpdateApproachEnemySettings(float newspawnInterval = -1, float newSpeed = -1, float newDistance = -1);

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void ShootInk(FVector2D pos, APlayerController* PC);

    // 清理所有ApproachEnemy
    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void ClearAllApproachEnemies();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/ApproachEnemy")
    void RecycleApproachEnemy(AApproachEnemyCharacter* target);



#pragma endregion

#pragma region AdEnemy

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/AdEnemy")
    void StartAdEnemyGenerator();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/AdEnemy")
    void StopAdEnemyGenerator();

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/AdEnemy")
    void ClearAllAdEnemies();

    void RecycleAdEnemy(AAdEnemyAIController* target);



    UFUNCTION(BlueprintCallable, Category = "EnemyManager")
    void SetSpawnActive(FEnemyLocationInfo& infoRef, bool active);

    



#pragma endregion




#pragma region AdEnemySettings

    UFUNCTION(BlueprintCallable, Category = "EnemyManager/AdEnemy")
    void AddInfo(FEnemyLocationInfo info);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/AdEnemy")
    TSubclassOf<AAdEnemyCharacter> AdEnemyBPClass;  // 用于在编辑器中指定要生成的蓝图Character类[10](@ref)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/AdEnemy")
    int32 ObjectPoolCount = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/AdEnemy")
    float AdEnemySpawnInterval = 5.0f;  // 生成间隔，默认为5秒

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnemyLocationInfo> AdEnemyInfos;

#pragma endregion

#pragma region ApproachEnemySettings

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy")
    FActiveInk InkSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy")
    TSubclassOf<AApproachEnemyCharacter> ApproachEnemyBPClass;  // 新的敌人蓝图类

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy")
    int32 MaxApproachEnemyCount = 6;  // 最大ApproachEnemy数量

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy", meta = (AllowPrivateAccess = "true"))
    float ApEnemyInitialDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy", meta = (AllowPrivateAccess = "true"))
    float ApEnemyApproachSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy", meta = (AllowPrivateAccess = "true"))
    float ApEnemyAttackDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnemyManager/ApproachEnemy")
    float ApproachEnemySpawnInterval = 2.0f;

#pragma endregion






private:
    UObjectPoolManager* PoolManager;

    UOldManEnemyManager();
    bool _hasGeneAdOnce = false;

    void GenerateAdEnemy();
    TArray<AAdEnemyAIController*> AdEnemyControls;
    TMap<int32, int32> _AdEnemySpawnCounts;
    static int32 nextID;
    FString _timerID_AdEnemy;





    void GenerateApproachEnemy();
    int32 CurrentApEnemyCount = 0;
    TArray<AApproachEnemyCharacter*> ApproachEnemies;
    FString m_timerID_ApproachEnemy;

    

  
    AOldManHUD* OldManHUD;


};
