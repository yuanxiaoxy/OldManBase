// Fill out your copyright notice in the Description page of Project Settings.


#include "AdEnemyManagerAgent.h"

// Sets default values
AAdEnemyManagerAgent::AAdEnemyManagerAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}



void AAdEnemyManagerAgent::DrawAdEnemyDebug(TArray<FEnemyLocationInfo> infos, float lastTime)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Log, TEXT("World无效"));
        return;  // World无效直接返回
    }
    // 只在编辑器模式绘制
    if (World->WorldType == EWorldType::Editor)
    {
        FlushPersistentDebugLines(World);
        FlushDebugStrings(World);
        for (int32 i = 0; i < infos.Num(); i++)
        {
            infos[i].DrawDebugInfo(World, lastTime);
        }
    }
}

