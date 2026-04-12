// Fill out your copyright notice in the Description page of Project Settings.


#include "BossGridManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "BossGrid.h"
// Sets default values
ABossGridManager::ABossGridManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

// Called when the game starts or when spawned
void ABossGridManager::BeginPlay()
{
	Super::BeginPlay();
	m_Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (GenerateCenter == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BossGridManager 的 GenerateCenter 是空指针！"));
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	else
	{
		m_GenerateVector = GenerateCenter->GetActorLocation();
	}

	if (!BossGridClass)
	{
		UE_LOG(LogTemp, Error, TEXT("BossGridManager 的 BossGridClass 是空指针！"));	
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	
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
			ABossGrid* NewGrid = GetWorld()->SpawnActor<ABossGrid>(BossGridClass);
			NewGrid->Initialize(i, j, m_GenerateVector);
			m_Grids[i].Add(NewGrid);
			if (!m_bSetExtent)
			{
				m_GridExtent = NewGrid->GridMeshComp->Bounds.BoxExtent;
			}
		}

	}
}




void ABossGridManager::RandomSetMap()
{
	//AllSetSafe();
	
	m_bHasSafeInRange = false;
	FIntPoint PlayerGridIndex = GetPlayerGridIndex();
	int32 PlayerGridX = PlayerGridIndex.X;
	int32 PlayerGridY = PlayerGridIndex.Y;
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
				Grid->SwitchToFlash(FlashDuriation);
				if (m_bHasSafeInRange)
					continue;
				if (IsInJumpRange(PlayerGridX, PlayerGridY, i, j))
				{
					m_bHasSafeInRange = true;
				}

				
			}
			else
			{
				Grid->SwitchToDanger(FlashDuriation);
			}
			
		}
	}
	if (!m_bHasSafeInRange)
	{
		ForceRandomSafeInJumpRange(PlayerGridIndex);
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

FIntPoint ABossGridManager::GetPlayerGridIndex()
{
	if (m_Player == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("BossGridManager 的 Player 是空指针！"));
		return FIntPoint(-1, -1);
	}
	FVector PlayerLoc = m_Player->GetActorLocation();

	float X_Offset = PlayerLoc.X - m_GenerateVector.X;
	float Y_Offset = PlayerLoc.Y - m_GenerateVector.Y;

	int32 GridX = FMath::FloorToInt32(X_Offset / m_GridExtent.X);
	int32 GridY = FMath::FloorToInt32(Y_Offset / m_GridExtent.Y);

	// 钳制在 0~6 之间，防止越界
	GridX = FMath::Clamp(GridX, 0, MapWidth - 1);
	GridY = FMath::Clamp(GridY, 0, MapHeight - 1);


	
	return FIntPoint(GridX, GridY);
}

void ABossGridManager::ForceRandomSafeInJumpRange(FIntPoint PlayerGrid)
{
	TArray<FIntPoint> CandidateGrids;

	for (int32 X = 0; X < MapWidth; X++)
	{
		for (int32 Y = 0; Y < MapHeight; Y++)
		{
			// 判断：这个格子是否在可跳范围内（斜跳也支持）
			if (IsInJumpRange(PlayerGrid.X, PlayerGrid.Y, X, Y))
			{
				CandidateGrids.Add(FIntPoint(X, Y));
			}
		}
	}

	// 3. 随机选一个格子
	if (CandidateGrids.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, CandidateGrids.Num() - 1);
		FIntPoint TargetGrid = CandidateGrids[RandomIndex];

		// 4. 把这个格子强制变成【安全】
		ABossGrid* TargetTile = m_Grids[TargetGrid.X][TargetGrid.Y];
		if (TargetTile)
		{
			TargetTile->SwitchToFlash(FlashDuriation);
			UE_LOG(LogTemp, Warning, TEXT("[强制保底] 格子 (%d,%d) 已设为安全！"), TargetGrid.X, TargetGrid.Y);
		}
	}
}