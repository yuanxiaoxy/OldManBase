// OldManMobileCamera.cpp
// 实现 Boss 的攻击发射逻辑。

#include "Boss/OldManMobileCamera.h"
#include "Boss/OldManHaloAttack.h"
#include "Boss/OldManBeamAttack.h"
#include "Engine/World.h"

AOldManMobileCamera::AOldManMobileCamera()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AOldManMobileCamera::BeginPlay()
{
	Super::BeginPlay();
	if (!HaloClass) UE_LOG(LogTemp, Error, TEXT("Boss_光圈攻击预制体不存在"));
	if (!BeamClass) UE_LOG(LogTemp, Error, TEXT("Boss_光束攻击预制体不存在"));

	MonoManager = UMonoManager::GetMonoManager();
	CurrentPhase = 0;

	bIsWaitingForHit = false;
	CurrentBullet = nullptr;
	StartCooldown();
}

void AOldManMobileCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOldManMobileCamera::Attack()
{
	// 如果正在等待当前子弹命中，或者老人头无效，则不发射新子弹
	if (bIsWaitingForHit || !OldManHead) return;

	// 根据当前阶段选择攻击方式
	//判断当前阶段不为End
	if (OldManHead->CurPhaseTYpe != EOldManHeadType::end)
	{
		switch (OldManHead->PhaseInfos[OldManHead->Index].AttactType)
		{
		case EOldManAttactType::Halo:
			FireHalo();
			break;
		case EOldManAttactType::Beam:
			FireBeam();
			break;
		}
	}
	else GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("Boss_End阶段attack")));
}

void AOldManMobileCamera::FireHalo()
{
	UE_LOG(LogTemp, Display, TEXT("Boss_FireHalo"));
	if (!HaloClass || !OldManHead) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AOldManHaloAttack* Halo = GetWorld()->SpawnActor<AOldManHaloAttack>(HaloClass, SpawnLocation, SpawnRotation, SpawnParams);
	Halo->MoveSpeed = OldManHead->PhaseInfos[OldManHead->Index].MoveSpeed;
	if (Halo)
	{
		Halo->TargetHead = OldManHead;
		Halo->AddProgress = OldManHead->PhaseInfos[OldManHead->Index].Progress;
		Halo->MoveSpeed = OldManHead->PhaseInfos[OldManHead->Index].MoveSpeed;
		// 绑定销毁事件，以便在子弹销毁时重置等待状态
		Halo->OnDestroyed.AddDynamic(this, &AOldManMobileCamera::OnBulletDestroyed);

		CurrentBullet = Halo;
		bIsWaitingForHit = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss_FireHalo: 生成失败"));
	}
}

void AOldManMobileCamera::FireBeam()
{
	UE_LOG(LogTemp, Display, TEXT("Boss_FireBeam"));
	if (!BeamClass || !OldManHead) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AOldManBeamAttack* Beam = GetWorld()->SpawnActor<AOldManBeamAttack>(BeamClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Beam)
	{
		Beam->TargetHead = OldManHead;
		Beam->ContinueTime = OldManHead->PhaseInfos[OldManHead->Index].ContinueTime;
		Beam->AddProgress = OldManHead->PhaseInfos[OldManHead->Index].Progress;
		Beam->MoveSpeed = OldManHead->PhaseInfos[OldManHead->Index].MoveSpeed;

		Beam->OnDestroyed.AddDynamic(this, &AOldManMobileCamera::OnBulletDestroyed);

		CurrentBullet = Beam;
		bIsWaitingForHit = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss_FireBeam: 生成失败"));
	}
}

void AOldManMobileCamera::ReadyAttack()
{
	// 保留空实现，供扩展
}

void AOldManMobileCamera::OnPhaseChanged(int32 NewPhase)
{
	CurrentPhase = NewPhase;

	// 移除旧的定时器，重新开始倒计时（使用新阶段的冷却时间）
	if (MonoManager)
	{
		// 假设 MonoManager 有 ClearTimer 方法，用于移除指定名称的定时器
		MonoManager->ClearTimer("Attack");
	}
	StartCooldown();

}

void AOldManMobileCamera::OnBulletDestroyed(AActor* DestroyedActor)
{
	// 只有当销毁的正是当前等待的子弹时，才重置状态
	if (DestroyedActor == CurrentBullet)
	{
		CurrentBullet = nullptr;
		bIsWaitingForHit = false;
		// 定时器仍在运行，下一次 Attack 调用时（倒计时结束）会发射新子弹
		// 移除旧的定时器，重新开始倒计时（使用新阶段的冷却时间）
		if (MonoManager)
		{
			// 假设 MonoManager 有 ClearTimer 方法，用于移除指定名称的定时器
			MonoManager->ClearTimer("Attack");
		}
		StartCooldown();
	}
}

void AOldManMobileCamera::StartCooldown()
{
	if (MonoManager && OldManHead && OldManHead->PhaseInfos.IsValidIndex(CurrentPhase))
	{
		float Cooldown = OldManHead->PhaseInfos[CurrentPhase].AttackCooldown;

		// 使用 MonoManager 的 SetInterval 设置循环定时器，但通过 bIsWaitingForHit 控制实际发射
		// 注意：SetInterval 会每 Cooldown 秒调用一次 Attack，但 Attack 内部会检查是否可发射
		MonoManager->SetInterval(Cooldown, "Attack", this, &AOldManMobileCamera::Attack);
	}
}