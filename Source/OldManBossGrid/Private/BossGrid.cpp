// Fill out your copyright notice in the Description page of Project Settings.

#include "BossGrid.h"


// Sets default values
ABossGrid::ABossGrid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	
}

// Called when the game starts or when spawned
void ABossGrid::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ABossGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentGridState == EGridState::Flashing)
	{
		// 倒计时

		FlashSwitchTimer += DeltaTime;

		// 每 0.2 秒切一次材质
		if (FlashSwitchTimer >= FlashFrequency)
		{
			FlashSwitchTimer = 0.f;

			// 交替切换安全 / 危险材质
			if (GridMeshComp->GetMaterial(0) == SafeMaterial)
				GridMeshComp->SetMaterial(0, DangerMaterials[0]);
			else
				GridMeshComp->SetMaterial(0, SafeMaterial);
		}	

	}

}


void ABossGrid::Initialize()
{

	if (GridMeshComp == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GridMeshComp 是空指针！"), *GetName());
		return;
	}

	if (SafeMaterial == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SafeMaterial 是空指针！"), *GetName());
	}

	if (DangerMaterials.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DangerMaterials 是空列表!"), *GetName());
	}

	GridMeshComp->SetMaterial(0, SafeMaterial);

	// 2. 开启碰撞 + 设置为 Overlap 模式
	GridMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GridMeshComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	// 3. 订阅（绑定）重叠事件 —— 这就是你要的！
	GridMeshComp->OnComponentBeginOverlap.AddDynamic(this, &ABossGrid::OnGridBeginOverlap);
	GridMeshComp->OnComponentEndOverlap.AddDynamic(this, &ABossGrid::OnGridEndOverlap);
}

void ABossGrid::SwitchToDanger()
{
	CurrentGridState = EGridState::Danger;
	int32 randomIndex = FMath::RandRange(0, DangerMaterials.Num() - 1);
	UMaterialInterface* next = DangerMaterials[randomIndex];
	GridMeshComp->SetMaterial(0, next);
}

void ABossGrid::SwitchToSafe()
{
	CurrentGridState = EGridState::Safe;
	GridMeshComp->SetMaterial(0, SafeMaterial);
}

void ABossGrid::SwitchToFlash(int32 FlashTime = 0)
{
	CurrentGridState = EGridState::Flashing;
}

void ABossGrid::OnGridBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (CurrentGridState != EGridState::Danger)
	{
		return;
	}

}

void ABossGrid::OnGridEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (CurrentGridState != EGridState::Danger)
	{
		return;
	}

}

void ABossGrid::SetPos(int32 X, int32 Y)
{
	GridX = X;
	GridY = Y;
}	


