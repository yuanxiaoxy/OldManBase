// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyPathDebugger.h"

void AEnemyPathDebugger::DrawEnemyDebugInfo(TArray<FEnemyLocationInfo> infos, float lastTime)
{
#if WITH_EDITOR
    UWorld* world = GetWorld();
    // 添加世界类型检查，确保在纯编辑器模式下也执行
    if (!world || (world->WorldType != EWorldType::Editor && world->WorldType != EWorldType::PIE))
        return;

    for (const FEnemyLocationInfo& info : infos)
    {
        info.DrawDebugInfo(world, lastTime);
    }
#endif
}

bool AEnemyPathDebugger::ShouldTickIfViewportsOnly() const
{
    return true; // 关键：允许在编辑器视口中Tick
}