// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManEnemyManager.h"
#include "AdEnemyAIController.h"
#include "AdEnemyStateTypes.h"

template<>
UOldManEnemyManager* TSingleton<UOldManEnemyManager>::SingletonInstance = nullptr;



// Sets default values
UOldManEnemyManager::UOldManEnemyManager()
{

}

void UOldManEnemyManager::InitializeSingleton()
{

}
void UOldManEnemyManager::NotifyMonstersTracking()
{
    for (AAdEnemyAIController* enemy : Enemys)
    {
        if (enemy != nullptr && enemy->GetCurrentState() != EAdMonsterState::Tracking) // 重要的空指针检查！
        {
            enemy->ChangeState(EAdMonsterState::Tracking);
        }
    }
}



