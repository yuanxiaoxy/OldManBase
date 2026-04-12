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
		if (!GridMeshComp || !SafeMaterial || DangerMaterials.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] 闪烁失败：缺少材质或组件！"), *GetName());
			SwitchToSafe(); // 出错了强制切回安全
			return;
		}



		// 倒计时

		FlashDurationTimer -= DeltaTime;
		FlashSwitchTimer += DeltaTime;
		// 每 0.2 秒切一次材质
		if (FlashSwitchTimer >= FlashFrequency)
		{
			FlashSwitchTimer = 0.f;

			// 安全判断：获取当前材质
			UMaterialInterface* CurrentMat = GridMeshComp->GetMaterial(0);

			if (CurrentMat == SafeMaterial)
			{
				GridMeshComp->SetMaterial(0, DangerMaterials[0]);
			}
			else
			{
				GridMeshComp->SetMaterial(0, SafeMaterial);
			}
		}	
		if (FlashDurationTimer <= 0)
		{
			SwitchToSafe();
		}
	}

}


void ABossGrid::Initialize(int32 X, int32 Y, FVector generate)
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

	
	SetPos(X, Y, generate);
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
	FlashDurationTimer = 0.f;
	FlashSwitchTimer = 0.f;
	CurrentGridState = EGridState::Safe;
	GridMeshComp->SetMaterial(0, SafeMaterial);
}

void ABossGrid::SwitchToFlash(float FlashTime = 2)
{
	FlashDurationTimer = FlashTime;
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

void ABossGrid::SetPos(int32 X, int32 Y, FVector generate)
{
	GridX = X;
	GridY = Y;
	FVector BoxExtent = GridMeshComp->Bounds.BoxExtent;
	FVector NewLocation = generate + FVector(X * BoxExtent.X, Y * BoxExtent.Y , 0.f);
	SetActorLocation(NewLocation);
}	


