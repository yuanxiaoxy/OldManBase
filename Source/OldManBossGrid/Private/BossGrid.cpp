// Fill out your copyright notice in the Description page of Project Settings.

#include "BossGrid.h"


// Sets default values
ABossGrid::ABossGrid()
{
	PrimaryActorTick.bCanEverTick = true;
	GridMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridMeshComp"));
	RootComponent = GridMeshComp;
	GridMeshComp->SetMobility(EComponentMobility::Movable);
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

}


void ABossGrid::Initialize(int32 X, int32 Y, FVector generate, int32 delta)
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

	
	SetPos(X, Y, generate, delta);
}

void ABossGrid::SwitchToDanger(float FlashTime)
{
	if(CurrentGridState == EGridState::Danger)
	{
		return;
	}

	int32 randomIndex = FMath::RandRange(0, DangerMaterials.Num() - 1);
	UMaterialInterface* nextMat = DangerMaterials[randomIndex];

	// 延迟 FlashTime 秒后执行【真正的切换逻辑】
	FTimerDelegate TimerDelegate;

	// 绑定：要延迟执行的函数 + 传入参数
	TimerDelegate.BindUObject(this, &ABossGrid::OnSwitchToDangerDelayed, nextMat);

	// 设置定时器
	GetWorldTimerManager().SetTimer(
		m_TimerHandle_SwitchDanger,	// 定时器句柄
		TimerDelegate,			// 要执行的函数
		FlashTime,				// 延迟时间
		false					// 不循环
	);
}

void ABossGrid::OnSwitchToDangerDelayed(UMaterialInterface* nextMat)
{
	CurrentGridState = EGridState::Danger;
	GridMeshComp->SetMaterial(0, nextMat);
}	

void ABossGrid::SwitchToSafe()
{
	// 停止所有计时器
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashSwitch);
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashDuration);

	m_bIsFlashing = false;
	CurrentGridState = EGridState::Safe;
	GridMeshComp->SetMaterial(0, SafeMaterial);
}

void ABossGrid::SwitchToFlash(float FlashTime /*= 2*/)
{
	if (!GridMeshComp || !SafeMaterial || DangerMaterials.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 闪烁失败：缺少材质或组件！"), *GetName());
		SwitchToSafe();
		return;
	}

	// 先清掉旧计时器，防止重复触发
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashSwitch);
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashDuration);

	CurrentGridState = EGridState::Flashing;
	m_bIsFlashing = true;

	// 开始循环切换材质
	GetWorldTimerManager().SetTimer(
		TimerHandle_FlashSwitch,
		FTimerDelegate::CreateUObject(this, &ABossGrid::ToggleFlashMaterial),
		FlashFrequency, // 每XX秒闪一次
		true // 循环
	);

	// 设置总时长，时间到停止
	GetWorldTimerManager().SetTimer(
		TimerHandle_FlashDuration,
		FTimerDelegate::CreateUObject(this, &ABossGrid::SwitchToSafe),
		FlashTime,
		false
	);
}

void ABossGrid::ToggleFlashMaterial()
{
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

void ABossGrid::SetPos(int32 X, int32 Y, FVector generate, int32 delta)
{
	GridX = X;
	GridY = Y;
	UStaticMesh* Mesh = GridMeshComp->GetStaticMesh();
	if (Mesh == nullptr) return; // 防止空指针

	// 2. 获取视觉模型的本地包围盒（真实大小，不带任何缩放）
	FVector MeshLocalExtent = Mesh->GetBounds().BoxExtent;

	// 3. 应用组件缩放，得到画面上的真实大小
	FVector BoxExtent = MeshLocalExtent * GridMeshComp->GetComponentScale();
	// 格子宽度 = 2个半尺寸（完整大小） + 间距
	float GridWidth = BoxExtent.X * 2 + delta;
	float GridHeight = BoxExtent.Y * 2 + delta;

	FVector Offset(
		X * GridWidth,
		Y * GridHeight,
		0.f
	);

	if (GetAttachParentActor())
	{
		SetActorRelativeLocation(generate + Offset);
	}
	else
	{
		SetActorLocation(generate + Offset);
	}
}	


