// Fill out your copyright notice in the Description page of Project Settings.

#include "BossGrid.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

namespace
{
	TSet<TWeakObjectPtr<AActor>> GDangerEffectActors;

	bool IsPlayerActor(const AActor* Actor)
	{
		const APawn* Pawn = Cast<APawn>(Actor);
		return Pawn && Pawn->IsPlayerControlled();
	}

	bool TryEnterDanger(AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}
		const TWeakObjectPtr<AActor> Key = Actor;
		if (GDangerEffectActors.Contains(Key))
		{
			return false;
		}
		GDangerEffectActors.Add(Key);
		return true;
	}

	void LeaveDanger(AActor* Actor)
	{
		if (!Actor)
		{
			return;
		}
		GDangerEffectActors.Remove(TWeakObjectPtr<AActor>(Actor));
	}
}

// Sets default values
ABossGrid::ABossGrid()
{
	PrimaryActorTick.bCanEverTick = true;
	GridMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GridMeshComp"));
	RootComponent = GridMeshComp;
	GridMeshComp->SetMobility(EComponentMobility::Movable);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(RootComponent);
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxCollision->SetGenerateOverlapEvents(true);
	BoxCollision->SetBoxExtent(FVector(50.0f, 50.0f, 50.0f));
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

	if (SafeMaterials.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SafeMaterial 是空指针！"), *GetName());
	}

	if (DangerMaterials.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DangerMaterials 是空列表!"), *GetName());
	}

	for (int i = 0; i < SafeMaterials.Num(); i++)
	{
		GridMeshComp->SetMaterial(i, SafeMaterials[i]);
	}

	if (!BoxCollision)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] BoxCollision 是空指针！"), *GetName());
		return;
	}

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ABossGrid::OnGridBeginOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &ABossGrid::OnGridEndOverlap);

	
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
	for (int i = 0; i < DangerMaterials.Num(); i++)
	{
		GridMeshComp->SetMaterial(i, DangerMaterials[i]);
	}

	if (bPlayerOnGrid && PlayerActorOnGrid && IsPlayerActor(PlayerActorOnGrid))
	{
		if (TryEnterDanger(PlayerActorOnGrid))
		{
			OnPlayerInDanger(PlayerActorOnGrid);
		}
	}
}	

void ABossGrid::SwitchToSafe()
{
	// 停止所有计时器
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashSwitch);
	GetWorldTimerManager().ClearTimer(TimerHandle_FlashDuration);

	m_bIsFlashing = false;
	CurrentGridState = EGridState::Safe;
	for (int i = 0; i < SafeMaterials.Num(); i++)
	{
		GridMeshComp->SetMaterial(i, SafeMaterials[i]);
	}

	if (bPlayerOnGrid && PlayerActorOnGrid && IsPlayerActor(PlayerActorOnGrid))
	{
		LeaveDanger(PlayerActorOnGrid);
		OnPlayerInSafe(PlayerActorOnGrid);
	}
}

void ABossGrid::SwitchToFlash(float FlashTime /*= 2*/)
{
	if (!GridMeshComp || SafeMaterials.Num() == 0 || DangerMaterials.Num() == 0)
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

	if (CurrentMat == SafeMaterials[0])
	{
		for (int i = 0; i < DangerMaterials.Num(); i++)
		{
			GridMeshComp->SetMaterial(i, DangerMaterials[i]);
		}
	}
	else
	{
		for (int i = 0; i < SafeMaterials.Num(); i++)
		{
			GridMeshComp->SetMaterial(i, SafeMaterials[i]);
		}
	}
}


void ABossGrid::OnGridBeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPlayerActor(OtherActor))
	{
		return;
	}

	bPlayerOnGrid = true;
	PlayerActorOnGrid = OtherActor;

	if (CurrentGridState != EGridState::Danger)
	{
		LeaveDanger(OtherActor);
		OnPlayerInSafe(OtherActor);
		return;
	}

	if (TryEnterDanger(OtherActor))
	{
		OnPlayerInDanger(OtherActor);
	}
}

void ABossGrid::OnGridEndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsPlayerActor(OtherActor))
	{
		return;
	}

	if (OtherActor == PlayerActorOnGrid)
	{
		bPlayerOnGrid = false;
		PlayerActorOnGrid = nullptr;
	}

	if (CurrentGridState != EGridState::Danger)
	{
		return;
	}
	OnPlayerOutDanger(OtherActor);
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

	if (BoxCollision)
	{
		const float Extra = static_cast<float>(delta) * 0.5f;
		BoxCollision->SetBoxExtent(FVector(BoxExtent.X + Extra, BoxExtent.Y + Extra * 2, FMath::Max(BoxExtent.Z, 100.0f)));
	}

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


