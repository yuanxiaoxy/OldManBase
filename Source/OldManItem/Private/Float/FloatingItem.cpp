#include "Float/FloatingItem.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
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
			// 强制精确回到初始变换，消除任何浮点误差
			SetActorLocation(InitialLocation);
			SetActorRotation(InitialRotation);

			// 结束循环，分离玩家
			EndSinkRiseCycle();
			CurrentState = EFloatingItemState::Idle;

			// 重置浮动相位，使 Idle 状态的浮动从零偏移开始
			RunningTime = 0.0f;
		}
		else
		{
			CycleProgress = NewProgress;
			float T = FMath::Sin(CycleProgress * PI);
			FVector SinkOffset = FVector(0, 0, -SinkDepth) * T;
			SetActorLocation(InitialLocation + SinkOffset);

			float Shake = FMath::Sin(CycleProgress * PI * 2.0f) * 2.0f;
			FRotator ExtraRot = FRotator(Shake, Shake * 0.5f, Shake * 0.3f);
			SetActorRotation(InitialRotation + ExtraRot);
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

	StartSinkRiseCycle(Player);
}

void AFloatingItem::StartSinkRiseCycle(AActor* PlayerActor)
{
	if (!PlayerActor) return;

	AttachedPlayer = PlayerActor;
	AttachedPlayer->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	CurrentState = EFloatingItemState::SinkRiseCycle;
	CycleProgress = 0.0f;
	RunningTime = 0.0f;          // 重置浮动相位，避免 Idle 历史偏移干扰
	SetActorLocation(InitialLocation);
	SetActorRotation(InitialRotation);
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