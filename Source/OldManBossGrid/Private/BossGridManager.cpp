// Fill out your copyright notice in the Description page of Project Settings.


#include "BossGridManager.h"
#include "BossGrid.h"
// Sets default values
ABossGridManager::ABossGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABossGridManager::BeginPlay()
{
	Super::BeginPlay();
	
	
}

// Called every frame
void ABossGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABossGridManager::GenerateMap()
{
	m_Grids.Empty();
	for (int32 i = 0; i < MapWidth; i++)
	{
		m_Grids.Add(TArray<ABossGrid*>());
		for (int32 j = 0; j < MapHeight; j++)
		{
			// UE 生成 Actor 的正确写法
			ABossGrid* NewGrid = GetWorld()->SpawnActor<ABossGrid>(ABossGrid::StaticClass());
			NewGrid->SetPos(i, j);
			NewGrid->Initialize();
			m_Grids[i].Add(NewGrid);
		}

	}
}




void ABossGridManager::RandomSetMap()
{
	//AllSetSafe();
	
	m_hasSafeInRange = false;

	//	先随机设置危险格子
	for (int32 i = 0; i < m_Grids.Num(); i++)
	{
		for (int32 j = 0; j < m_Grids[i].Num(); j++)
		{
			// 80% 危险，20% 安全，难度很高
			bool bDanger = FMath::RandRange(0, 9) < DangerProbability;
			ABossGrid* Grid = m_Grids[i][j];
			// 作为安全格子要检查是否在玩家跳跃范围内，并且设置m_hasSafeInRange
			if (!bDanger)
			{
				if (m_hasSafeInRange)
					continue;

				
			}
			else
			{

			}
			
		}
	}


}

bool ABossGridManager::IsInJumpRange(int32 PlayerX, int32 PlayerY, int32 TargetX, int32 TargetY)
{
	int32 DX = FMath::Abs(TargetX - PlayerX);
	int32 DY = FMath::Abs(TargetY - PlayerY);

	// 最大偏移不超过N → 可以跳（斜跳也包含）
	return DX <= MaxJumpGridCount && DY <= MaxJumpGridCount;
	
}



void ABossGridManager::AllSetSafe()
{
	for(int32 i = 0; i < m_Grids.Num(); i++)
	{
		for(int32 j = 0; j < m_Grids[i].Num(); j++)
		{
			ABossGrid* Grid = m_Grids[i][j];
			if(Grid->bPlayerOnGrid)
				continue;
			Grid->SwitchToSafe();
		}
	}
}
