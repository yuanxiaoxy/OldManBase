// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManEnemyManager.h"
#include "AdEnemyAIController.h"
#include "AdEnemyStateTypes.h"
#include "MonoManager/MonoManager.h"

template<>
UOldManEnemyManager* TSingleton<UOldManEnemyManager>::SingletonInstance = nullptr;



// Sets default values
UOldManEnemyManager::UOldManEnemyManager()
{
    
}

void UOldManEnemyManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("敌人管理器初始化 start"));
    PoolManager = UObjectPoolManager::GetInstance();
    PoolManager->InitializeSingleton();
    PoolManager->Preload(EnemyBlueprintClass, 5);
    // 验证蓝图类是否设置
    if (EnemyBlueprintClass)
    {
        // 立即生成一批敌人
        GenerateEnemy();
        _hasInitialze = true;
        //FTimerSimpleDelegate timerDelegate;
        //timerDelegate.BindUFunction(this, "GenerateEnemy");
        FString timerID = UMonoManager::GetInstance()->
            SetInterval(SpawnInterval, this, &UOldManEnemyManager::GenerateEnemy);
        // 设置定时器，每隔SpawnInterval秒生成一次[1](@ref)
        //if (UWorld* World = GetWorld())
        //{
        //    World->GetTimerManager().SetTimer(
        //        EnemySpawnTimerHandle,
        //        this,
        //        &UOldManEnemyManager::GenerateEnemy,
        //        SpawnInterval,
        //        true  // 循环执行
        //    );
        //}

        //UE_LOG(LogTemp, Log, TEXT("Enemy spawn system initialized with interval: %f seconds"), SpawnInterval);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyBlueprintClass is not set - enemy spawn system cannot start"));
    }
}


void UOldManEnemyManager::NotifyMonstersTracking()
{
    for (AAdEnemyAIController* enemy : Enemys)
    {
        if (enemy != nullptr && !enemy->hasTracked) // 重要的空指针检查！
        {
            enemy->hasTracked = true;
            enemy->ChangeState(EAdMonsterState::Tracking);
        }
    }
}





void UOldManEnemyManager::AddInfos(FEnemyLocationInfo info)
{
    EnemyInfos.Add(info);
}


void UOldManEnemyManager::GenerateEnemy()
{
    if (!EnemyBlueprintClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyBlueprintClass is not set in UOldManEnemyManager!"));
        return;
    }

    

    // 遍历所有敌人生成信息
    for (int32 i = 0; i < EnemyInfos.Num(); ++i)
    {

        const FEnemyLocationInfo& EnemyInfo = EnemyInfos[i];
        if (_hasInitialze && EnemyInfo.bIsGenerateOnce)
            continue;
        if (!EnemyInfo.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("EnemyInfo at index %d is invalid, skipping spawn"), i);
            continue;
        }

        if (!EnemyInfo.isActive)
            continue;
        FVector SpawnLocation = EnemyInfo.SpawnPoint->GetActorLocation();
        FRotator SpawnRotation = FRotator::ZeroRotator;
        AActor* SpawnedEnemy = PoolManager->Spawn
        (
            EnemyBlueprintClass,  // 参数之间用逗号分隔
            SpawnLocation,
            SpawnRotation,
            nullptr
        );

        if (SpawnedEnemy)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully spawned enemy at location: %s"), *SpawnLocation.ToString());

            // 调用敌人的初始化方法
            IEnemyInitializationInterface::Execute_InitializeEnemy(SpawnedEnemy, EnemyInfo);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy at location: %s"), *SpawnLocation.ToString());
        }
    }
}


void UOldManEnemyManager::SetSpawnActive(FEnemyLocationInfo& infoRef, bool active)
{
    infoRef.isActive = active;
}





//UWorld* UOldManEnemyManager::GetWorld() const
//{
//    // 通过Outer链获取World上下文
//    if (HasAnyFlags(RF_ClassDefaultObject))
//    {
//        // 如果是CDO（类默认对象），返回nullptr
//        return nullptr;
//    }
//
//    // 通过Outer链向上查找World
//    return GetOuter() ? GetOuter()->GetWorld() : nullptr;
//}
