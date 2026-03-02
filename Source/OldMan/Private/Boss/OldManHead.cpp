// OldManHead.cpp
// 实现老人头的所有逻辑。

#include "Boss/OldManHead.h"
#include "Engine/World.h"
#include "Boss/OldManMobileCamera.h"
#include "Boss/OldManControlStick.h"
#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

AOldManHead::AOldManHead()
{
	PrimaryActorTick.bCanEverTick = true;

	// 初始化旋转相关变量
	InitRot = FRotator::ZeroRotator;
	ShakeOffsetRot = FRotator::ZeroRotator;
	PlayerInputRot = FRotator::ZeroRotator;
	OffsetRot = FRotator::ZeroRotator;

	CurPhaseTYpe = EOldManHeadType::one;
	StickCanMove = false;
	DelayTime = 0;
	Timer = 0;
	IsOpenRange = false;
	TempAddProgress = 0;
	Index = 0;
	CanChangePhase = true;
	bIsBeamActive = false;
}

void AOldManHead::BeginPlay()
{
	Super::BeginPlay();

	// 记录初始旋转，并将 Roll 强制为 0
	InitRot = GetActorRotation();
	InitRot.Roll = 0.0f;

	// 检查必要引用是否已指定，若未指定则输出错误日志
	if (!MoblieCamera)
	{
		UE_LOG(LogTemp, Error, TEXT("Boss_MoblieCamera不存在"));
	}
	if (!ControlStick)
	{
		UE_LOG(LogTemp, Error, TEXT("Boss_ControlStick不存在"));
	}
}

void AOldManHead::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
#pragma region 仅测试，记得删
	/*if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		float Forward = 0;
		float Right = 0;
		if (PC->IsInputKeyDown(EKeys::W))Forward += 1;
		if (PC->IsInputKeyDown(EKeys::S))Forward -= 1;
		if (PC->IsInputKeyDown(EKeys::A))Right -= 1;
		if (PC->IsInputKeyDown(EKeys::D))Right += 1;

		FVector Direction = FVector(Right, Forward, 0.0f);
		ApplyInput(Direction);
		
	}*/

#pragma endregion

	// 根据当前阶段控制行为和权限
	switch (CurPhaseTYpe)
	{
	case EOldManHeadType::one:
		RandomShake(DeltaTime);   // 阶段1：随机晃动
		StickCanMove = true;      // 允许摇杆控制
		break;
	case EOldManHeadType::two:
		// 阶段2：关闭晃动和输入
		ShakeOffsetRot = FRotator::ZeroRotator;
		PlayerInputRot = FRotator::ZeroRotator;
		StickCanMove = false;
		break;
	case EOldManHeadType::three:
		RandomShake(DeltaTime);   // 阶段3：再次晃动
		StickCanMove = true;
		break;
	default:
		break;
	}

	// 从摇杆获取输入并应用旋转（如果允许）
	if (StickCanMove && ControlStick)
	{
		FVector InputDir = ControlStick->GetWorldDirection(); // 获取归一化方向
		if (!InputDir.IsNearlyZero())
		{
			ApplyInput(InputDir);
		}
		else
		{
			// 摇杆归零时，清零输入偏移（头部自动回正）
			PlayerInputRot = FRotator::ZeroRotator;
		}
	}

	// 计算总旋转偏移 = 晃动偏移 + 输入偏移
	OffsetRot = ShakeOffsetRot + PlayerInputRot;

	// 限制 Yaw 和 Pitch 在最大范围内，Roll 强制为 0
	OffsetRot.Yaw = FMath::Clamp(OffsetRot.Yaw, -YawMax, YawMax);
	OffsetRot.Pitch = FMath::Clamp(OffsetRot.Pitch, -PitchMax, PitchMax);
	OffsetRot.Roll = 0.0f;

	// 应用旋转
	SetActorRotation(InitRot + OffsetRot);

	// 延迟范围检测（用于持续攻击如光束的多次判定）
	if (IsOpenRange)
	{
		Timer += DeltaTime;
		if (!IsInSuccessRange(GetActorRotation(), CurPhaseTYpe))
		{
			// 中途偏离成功范围，取消本次延迟判定
			DelayTime = 0;
			Timer = 0;
			TempAddProgress = 0;
			IsOpenRange = false;
		}
		else if (Timer >= DelayTime)
		{
			// 成功维持足够时间，增加进度
			AddProgress(TempAddProgress);
			DelayTime = 0;
			Timer = 0;
			TempAddProgress = 0;
			IsOpenRange = false;
		}
	}
}

void AOldManHead::ApplyInput(const FVector& Direction)
{
	if (!StickCanMove) return;

	// 将方向向量映射到旋转偏移：X 对应 Pitch（前后），Y 对应 Yaw（左右）
	float TargetPitch = Direction.X * StickRotateSpeed;
	float TargetYaw = Direction.Y * StickRotateSpeed;

	// 直接设置目标偏移（非累加，实现摇杆回中时头部回正）
	PlayerInputRot.Pitch += TargetPitch;
	PlayerInputRot.Yaw += TargetYaw;
	// Roll 不受影响
}

void AOldManHead::RandomShake(float DeltaTime)
{
	ShakeTimer += DeltaTime;

	// 使用 Perlin 噪声生成平滑随机值
	float Noise = FMath::PerlinNoise1D(ShakeTimer * ShakeFrequency);

	// 计算 Yaw 和 Pitch 的晃动偏移
	float ShakeYaw = Noise * ShakeAmplitude * YawRate;
	float ShakePitch = Noise * ShakeAmplitude * PitchRate;

	ShakeOffsetRot.Yaw = ShakeYaw;
	ShakeOffsetRot.Pitch = ShakePitch;
	ShakeOffsetRot.Roll = 0.0f;
}

void AOldManHead::Blink(float progress)
{
	// 播放眨眼动画（如有）并增加进度
	AddProgress(progress);
}

bool AOldManHead::IsInSuccessRange(const FRotator& Rotation, EOldManHeadType curType)
{
	UpdateIndex(curType);
	if (Index < 0 || Index >= PhaseInfos.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Boss_超出检测范围索引值"));
		return false;
	}
	const FPhaseInfo& Phase = PhaseInfos[Index];

	// 计算当前旋转相对于初始旋转的偏移（归一化处理）
	FRotator RotOffset = (Rotation - InitRot).GetNormalized();

	// 比较 Yaw 和 Pitch 的绝对值是否小于等于允许范围（Roll 忽略）
	return FRotatorLessAbs(RotOffset, Phase.RotRangeOffset);
}

void AOldManHead::OnAttackHit(float addProgress, float delayTime)
{
	if (delayTime > 0)
	{
		// 需要延迟判定（例如光束持续命中）
		DelayTime = delayTime;
		Timer = 0;
		IsOpenRange = true;
		TempAddProgress = addProgress;
	}
	else
	{
		// 立即判定
		if (IsInSuccessRange(GetActorRotation(), CurPhaseTYpe))
		{
			AddProgress(addProgress);
		}
		// 失败时无变化（可扩展失败减少逻辑）
	}
}

void AOldManHead::AddProgress(float addProgress)
{
	CurProgress += addProgress;
	UE_LOG(LogTemp, Display, TEXT("Boss_AddPro:%f"), CurProgress);

	if (CanChangePhase && CurProgress >= PhaseInfos[Index].PhaseThreshold)
	{
		EOldManHeadType newPhase = (EOldManHeadType)((uint8)CurPhaseTYpe + 1);
		ChangePhase(newPhase);
	}
	UpdateIndex(CurPhaseTYpe);
}

void AOldManHead::SubtractProgress(float progress)
{
	CurProgress = FMath::Max(0.0f, CurProgress - progress);
	// 如需降阶段逻辑，可在此实现
}

void AOldManHead::ChangePhase(EOldManHeadType newPhase)
{
	FString PhaseStrr;
	switch (newPhase)
	{
	case EOldManHeadType::one: PhaseStrr = TEXT("one"); break;
	case EOldManHeadType::three: PhaseStrr = TEXT("three"); break;
	case EOldManHeadType::end: PhaseStrr = TEXT("end"); break;
	case EOldManHeadType::two: PhaseStrr = TEXT("two"); break;
	default: PhaseStrr = TEXT("unknown"); break;
	}
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, FString::Printf(TEXT("当前阶段：%s"), *PhaseStrr));
	if (newPhase == EOldManHeadType::end)
		CanChangePhase = false;

	// 特殊条件：阶段2不能在光束攻击中切换
	if (newPhase == EOldManHeadType::two && bIsBeamActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot change to phase2 during beam attack"));
		return;
	}

	int32 OldPhase = (int32)CurPhaseTYpe;
	int32 NewPhase = (int32)newPhase;

	CurPhaseTYpe = newPhase;
	CurProgress = 0; // 重置进度

	// 广播事件
	OnPhaseChanged.Broadcast(OldPhase, NewPhase);

	// 更新 UI 文本
	if (PhaseText)
	{
		FString PhaseStr = UEnum::GetValueAsString(newPhase);
		PhaseText->SetText(FText::FromString(PhaseStr));
	}

	// 通知 Boss 阶段变化
	if (MoblieCamera)
	{
		MoblieCamera->OnPhaseChanged(NewPhase);
	}
}

bool AOldManHead::FRotatorLessAbs(const FRotator& left, const FRotator& right)
{
	// 仅比较 Yaw 和 Pitch
	return FMath::Abs(left.Yaw) <= FMath::Abs(right.Yaw) &&
		FMath::Abs(left.Pitch) <= FMath::Abs(right.Pitch);
}

void AOldManHead::UpdateIndex(EOldManHeadType curType)
{
	switch (curType)
	{
	case EOldManHeadType::one: Index = 0; break;
	case EOldManHeadType::two: Index = 1; break;
	case EOldManHeadType::three: Index = 2; break;
	default: Index = -2; break;
	}
}