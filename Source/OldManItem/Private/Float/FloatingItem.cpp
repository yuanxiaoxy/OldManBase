#include "Float/FloatingItem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "AudioManager/AudioManager.h"
#include "Math/UnrealMathUtility.h"

AFloatingItem::AFloatingItem()
{
	PrimaryActorTick.bCanEverTick = true;

	if (!MeshComponent)
	{
		MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
		RootComponent = MeshComponent;
	}

	if (MeshComponent)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
		MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		MeshComponent->SetGenerateOverlapEvents(true);
	}
}

void AFloatingItem::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
	InitialRotation = GetActorRotation();

	if (MeshComponent)
	{
		MeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AFloatingItem::OnMeshBeginOverlap);
	}
}

void AFloatingItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case EFloatingItemState::Idle:
	{
		RunningTime += DeltaTime;
		FVector FloatOffset = CalculateFloatOffset();
		FRotator FloatRot = CalculateFloatRotation();
		SetActorLocation(InitialLocation + FloatOffset);
		SetActorRotation(InitialRotation + FloatRot);
		break;
	}

	case EFloatingItemState::SinkRiseCycle:
	{
		float NewProgress = CycleProgress + DeltaTime / TotalCycleDuration;
		if (NewProgress >= 1.0f)
		{
			// 循环结束：精确回到起点（触发时的实际位置和旋转）
			SetActorLocation(StartLocation);
			SetActorRotation(StartRotation);

			// 恢复浮动相位，使 Idle 状态无缝衔接
			RunningTime = CachedRunningTime;

			// 分离玩家并回到浮动状态
			EndSinkRiseCycle();
			CurrentState = EFloatingItemState::Idle;
		}
		else
		{
			CycleProgress = NewProgress;
			// 正弦曲线：progress=0 -> 0, progress=0.5 -> 1, progress=1 -> 0
			float T = FMath::Sin(CycleProgress * PI);
			// 位置：从 StartLocation 线性插值到 SinkTargetLocation，再插值回来（由 T 自动完成）
			FVector CurrentLocation = FMath::Lerp(StartLocation, SinkTargetLocation, T);
			SetActorLocation(CurrentLocation);

			// 可选：在下沉-上浮过程中加入轻微的旋转摆动（基于正弦，终点回到 StartRotation）
			float Shake = FMath::Sin(CycleProgress * PI * 2.0f) * 2.0f;
			FRotator ExtraRot = FRotator(Shake, Shake * 0.5f, Shake * 0.3f);
			SetActorRotation(StartRotation + ExtraRot);
		}
		break;
	}
	}
}

void AFloatingItem::OnMeshBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* Player = Cast<ACharacter>(OtherActor);
	if (!Player) return;
	if (CurrentState != EFloatingItemState::Idle) return;

	// 只有下落时才触发（跳上来），避免地面走上也触发
	FVector PlayerVelocity = Player->GetVelocity();
	if (PlayerVelocity.Z >= 0.0f) return;

	UAudioManager::GetInstance()->PlaySound(this, "SFX_OnWaterFloatItem");

	StartSinkRiseCycle(Player);
}

void AFloatingItem::StartSinkRiseCycle(AActor* PlayerActor)
{
	if (!PlayerActor) return;

	// 记录触发时的实际位置、旋转和浮动相位
	StartLocation = GetActorLocation();
	StartRotation = GetActorRotation();
	CachedRunningTime = RunningTime;

	// 计算下沉最深点的位置（从起点向下偏移）
	SinkTargetLocation = StartLocation - FVector(0, 0, SinkDepth);

	// 附加玩家
	AttachedPlayer = PlayerActor;
	AttachedPlayer->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	// 初始化状态机
	CurrentState = EFloatingItemState::SinkRiseCycle;
	CycleProgress = 0.0f;

	// 注意：不改变 Actor 当前位置和旋转，直接开始运动，避免突变
}

void AFloatingItem::EndSinkRiseCycle()
{
	if (AttachedPlayer)
	{
		AttachedPlayer->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		AttachedPlayer = nullptr;
	}
}

FVector AFloatingItem::CalculateFloatOffset()
{
	float Time = RunningTime;
	float OffsetZ = 0.0f;
	OffsetZ += VerticalAmplitude * FMath::Sin(Time * VerticalSpeed1);
	OffsetZ += (VerticalAmplitude * 0.6f) * FMath::Sin(Time * VerticalSpeed2 + 1.2f);
	OffsetZ += (VerticalAmplitude * 0.3f) * FMath::Sin(Time * VerticalSpeed3 + 2.5f);

	float OffsetX = HorizontalAmplitude * FMath::Sin(Time * HorizontalSpeedX);
	float OffsetY = HorizontalAmplitude * FMath::Sin(Time * HorizontalSpeedY);

	return FVector(OffsetX, OffsetY, OffsetZ);
}

FRotator AFloatingItem::CalculateFloatRotation()
{
	float Time = RunningTime;
	float Pitch = RotateAmplitude * FMath::Sin(Time * RotateSpeedPitch);
	float Roll = RotateAmplitude * 0.8f * FMath::Sin(Time * RotateSpeedRoll + 0.7f);
	float Yaw = RotateAmplitude * 0.5f * FMath::Sin(Time * RotateSpeedYaw);
	return FRotator(Pitch, Yaw, Roll);
}