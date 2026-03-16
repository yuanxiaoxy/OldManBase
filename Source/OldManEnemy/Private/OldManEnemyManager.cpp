// Fill out your copyright notice in the Description page of Project Settings.

#include "OldManEnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "AdEnemyAIController.h"
#include "Math/UnrealMathUtility.h"
#include "AdEnemyStateTypes.h"
#include "MonoManager/MonoManager.h"
#include "OldManHUD.h"
#include "ApproachEnemyCharacter.h"


int32 UOldManEnemyManager::nextID = 0;

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
    PoolManager->Preload(AdEnemyBPClass, 10);
    PoolManager->Preload(ApproachEnemyBPClass, MaxApproachEnemyCount);
    
    StartAdEnemyGenerator();
    //StartApproachEnemyGenerator();
}






#pragma region AdEnemy

// 开始周期生成广告敌人
void UOldManEnemyManager::StartAdEnemyGenerator()
{
    if (!m_timerID_AdEnemy.IsEmpty())
        return;
    // 验证蓝图类是否设置
    if (AdEnemyBPClass)
    {
        // 立即生成一批敌人
        GenerateAdEnemy();
        m_hasGeneAdOnce = true;
        //FTimerSimpleDelegate timerDelegate;
        //timerDelegate.BindUFunction(this, "GenerateEnemy");
        m_timerID_AdEnemy = UMonoManager::GetInstance()->
            SetInterval(AdEnemySpawnInterval, this, &UOldManEnemyManager::GenerateAdEnemy);

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyBlueprintClass is not set - enemy spawn system cannot start"));
    }
}

// 停止生成广告敌人
void UOldManEnemyManager::StopAdEnemyGenerator()
{
    if (m_timerID_AdEnemy.IsEmpty())
        return;
    UMonoManager::GetInstance()->ClearTimer(m_timerID_AdEnemy);
    m_timerID_AdEnemy.Empty();
}

// 清理所有广告敌人
void UOldManEnemyManager::ClearAllAdEnemies()
{
    const TArray<AAdEnemyAIController*> ControllersToClear = AdEnemyControls;
    for (AAdEnemyAIController* Controller : ControllersToClear)
    {
        if (Controller && Controller->EnemyCharacter)
        {
            Controller->EnemyCharacter->OnDespawn_Implementation();
        }
    }

    for (TPair<int32, int32>& Pair : _AdEnemySpawnCounts)
    {
        Pair.Value = 0;
    }
}

// 通知其他广告敌人追击玩家
void UOldManEnemyManager::NotifyMonstersTracking()
{
    for (AAdEnemyAIController* enemy : AdEnemyControls)
    {
        if (enemy != nullptr && !enemy->hasTracked) // 重要的空指针检查！
        {
            enemy->hasTracked = true;
            enemy->ChangeState(EAdMonsterState::Tracking);
        }
    }
}

// 添加位置信息到列表
void UOldManEnemyManager::AddInfo(FEnemyLocationInfo info)
{
    int32 currID = nextID++;
    info.ID = currID;
    if (!_AdEnemySpawnCounts.Contains(currID))
        _AdEnemySpawnCounts.Add(currID, 0);
    else
        UE_LOG(LogTemp, Error, TEXT("AdEnemy的计数字典key值：%d 冲突"), currID);
    AdEnemyInfos.Add(info);
}

// 按照信息表生成一批广告敌人
void UOldManEnemyManager::GenerateAdEnemy()
{
    if (!AdEnemyBPClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyBlueprintClass is not set in UOldManEnemyManager!"));
        return;
    }



    // 遍历所有敌人生成信息
    for (int32 i = 0; i < AdEnemyInfos.Num(); ++i)
    {

        const FEnemyLocationInfo& EnemyInfo = AdEnemyInfos[i];
        if (m_hasGeneAdOnce && EnemyInfo.bIsGenerateOnce)
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
        int32 ID = EnemyInfo.ID;

        if (!_AdEnemySpawnCounts.Contains(ID)) 
            _AdEnemySpawnCounts.Add(ID, 0);
        // 检查出生点怪物数量
        if (_AdEnemySpawnCounts[ID] >= EnemyInfo.maxCount)
            continue;

        AActor* SpawnedEnemy = PoolManager->Spawn
        (
            AdEnemyBPClass,  // 参数之间用逗号分隔
            SpawnLocation,
            SpawnRotation,
            nullptr
        );
        AAdEnemyCharacter* AdEnemy = Cast<AAdEnemyCharacter>(SpawnedEnemy);
        if (AdEnemy)
        {
            UE_LOG(LogTemp, Log, TEXT("Successfully spawned enemy at location: %s"), *SpawnLocation.ToString());
            AdEnemy->InitializeEnemy_Implementation(EnemyInfo);
            AdEnemy->OnSpawn_Implementation();
            _AdEnemySpawnCounts[ID]++;
            AdEnemyControls.Add(AdEnemy->AIController);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy at location: %s"), *SpawnLocation.ToString());
        }
    }
}

// 回收广告敌人
void UOldManEnemyManager::RecycleAdEnemy(AAdEnemyAIController* target)
{
    if (!target || !target->EnemyCharacter)
        return;

    const int32 ID = target->EnemyCharacter->SpawnInfoID;
    if (_AdEnemySpawnCounts.Contains(ID) && _AdEnemySpawnCounts[ID] > 0)
    {
        _AdEnemySpawnCounts[ID]--;
    }

    PoolManager->Despawn(target->EnemyCharacter);
    AdEnemyControls.Remove(target);
}



#pragma endregion


#pragma region ApproachEnemy

// 开始周期生成屏幕敌人
void UOldManEnemyManager::StartApproachEnemyGenerator()
{
    if (!m_timerID_ApproachEnemy.IsEmpty())
        return;
    // 验证蓝图类是否设置
    if (ApproachEnemyBPClass)
    {
        UE_LOG(LogTemp, Log, TEXT("StartApproachEnemyGenerator()"));
        // 立即生成一批敌人
        GenerateApproachEnemy();
        //FTimerSimpleDelegate timerDelegate;
        //timerDelegate.BindUFunction(this, "GenerateEnemy");
        m_timerID_ApproachEnemy = UMonoManager::GetInstance()->
            SetInterval(ApproachEnemySpawnInterval, this, &UOldManEnemyManager::GenerateApproachEnemy);

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyBlueprintClass is not set - enemy spawn system cannot start"));
    }
}

// 停止生成屏幕敌人
void UOldManEnemyManager::StopApproachEnemyGenerator()
{
    if (m_timerID_ApproachEnemy.IsEmpty())
        return;
    UMonoManager::GetInstance()->ClearTimer(m_timerID_ApproachEnemy);
    m_timerID_ApproachEnemy.Empty();
}

void UOldManEnemyManager::UpdateApproachEnemySettings(float newSpawnInterval, float newSpeed , float newDistance )
{
    if (newSpawnInterval != -1)
    {
        StopApproachEnemyGenerator();
        ApproachEnemySpawnInterval = newSpawnInterval;
        StartApproachEnemyGenerator();
    }
    if (newSpeed != -1)
        ApEnemyApproachSpeed = newSpeed;
    if (newDistance != -1)
        ApEnemyInitialDistance = newDistance;
}

void UOldManEnemyManager::ShootInk(FVector2D pos, APlayerController* PC)
{
    
    
    if (!PC)
    {

        UE_LOG(LogTemp, Warning, TEXT("UOldManEnemyManager::ShootInk: Failed to get PlayerController via Input."));
        return;
    }
        
    if (!OldManHUD)
    {
        OldManHUD = Cast<AOldManHUD>(PC->GetHUD());
    }
    
    
    if (InkTextures.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("喷墨贴图未设置"))
    }
    else
    {
        FActiveInk NewInk = InkSettings;
        NewInk.NormalizedPosition = pos;
        int32 randomIdx = FMath::RandRange(0, InkTextures.Num() - 1);
        NewInk.Texture = InkTextures[randomIdx];
        OldManHUD->AddInk(NewInk);
    }
    
   
    
}

// 清理所有屏幕敌人
void UOldManEnemyManager::ClearAllApproachEnemies()
{
    const TArray<AApproachEnemyCharacter*> EnemiesToClear = ApproachEnemies;
    for (AApproachEnemyCharacter* Enemy : EnemiesToClear)
    {
        if (Enemy)
        {
            Enemy->Recycle();
        }
    }

    ApproachEnemies.Empty();
    CurrentApEnemyCount = 0;
}

// 生成单只屏幕敌人
void UOldManEnemyManager::GenerateApproachEnemy()
{
    if (CurrentApEnemyCount >= MaxApproachEnemyCount) return;

    UE_LOG(LogTemp, Log, TEXT("GenerateApproachEnemy()"))
    AActor* SpawnedEnemy = PoolManager->Spawn(
        ApproachEnemyBPClass,  // 这里会自动转换为 TSubclassOf<AActor>
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        nullptr
    );
    if (SpawnedEnemy)
    {
        // 转换为具体类型
        AApproachEnemyCharacter* ApproachEnemy = Cast<AApproachEnemyCharacter>(SpawnedEnemy);
        if (ApproachEnemy)
        {
            FVector2D ScreenPos = FVector2D(FMath::FRandRange(0.2f, 0.8f), FMath::FRandRange(0.1f, 0.4f));
            ApproachEnemy->InitializeEnemy(ScreenPos, 
                ApEnemyAttackDistance, 
                ApEnemyApproachSpeed,
                ApEnemyInitialDistance,
                FlashDistance);
            ApproachEnemies.Add(ApproachEnemy);
            CurrentApEnemyCount++;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to Cast 'Actor' to 'ApproachEnemy' at location"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn ApproachEnemy at location"));
    }

}

// 回收屏幕敌人
void UOldManEnemyManager::RecycleApproachEnemy(AApproachEnemyCharacter* target)
{
    CurrentApEnemyCount--;
    PoolManager->Despawn(target);
    ApproachEnemies.Remove(target);
}

#pragma endregion


void UOldManEnemyManager::SetSpawnActive(FEnemyLocationInfo& infoRef, bool active)
{
    infoRef.isActive = active;
}





