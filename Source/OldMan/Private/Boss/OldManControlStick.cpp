// OldManControlStick.cpp
// 实现可拖拽摇杆的逻辑。

#include "Boss/OldManControlStick.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AOldManControlStick::AOldManControlStick()
{
	PrimaryActorTick.bCanEverTick = true;
	InputDir = FVector::ZeroVector;
	bIsBeingDragged = false;
	bInAutoBack = false;
	MovementAlpha = 1.0f;
	CurrentOffset = FVector::ZeroVector;
	TargetOffset = FVector::ZeroVector;
}

void AOldManControlStick::BeginPlay()
{
	Super::BeginPlay();

	if (!StickHead)
	{
		UE_LOG(LogTemp, Error, TEXT("Boss_ControlStick: StickHead 未指定！"));
		return;
	}

	// 记录摇杆头初始位置（中心点）
	InitPos = StickHead->GetActorLocation();

	// 确保摇杆头初始位置正确
	StickHead->SetActorLocation(InitPos);
	CurrentOffset = FVector::ZeroVector;
	TargetOffset = FVector::ZeroVector;
}

void AOldManControlStick::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);




	// 平滑移动：从当前位置向目标位置插值
	if (MovementAlpha < 1.0f)
	{
		MovementAlpha = FMath::Min(MovementAlpha + DeltaTime * 8.0f, 1.0f);
		CurrentOffset = FMath::Lerp(CurrentOffset, TargetOffset, MovementAlpha);
		StickHead->SetActorLocation(InitPos + CurrentOffset);
	}

	// 自动回正逻辑：松开后自动回到中心
	if (bInAutoBack)
	{
		AutoBackTimer += DeltaTime * AutoBackSpeed;
		float Alpha = FMath::Min(AutoBackTimer, 1.0f);
		CurrentOffset = FMath::Lerp(AutoBackStartOffset, FVector::ZeroVector, Alpha);
		StickHead->SetActorLocation(InitPos + CurrentOffset);

		if (Alpha >= 1.0f)
		{
			// 回正完成
			bInAutoBack = false;
			CurrentOffset = FVector::ZeroVector;
			TargetOffset = FVector::ZeroVector;
			MovementAlpha = 1.0f;
			SetActorTickEnabled(false);  // 无拖拽时停止Tick以节省性能
		}
	}

	// 根据当前偏移更新归一化输入方向
	UpdateInputDir();

	// 调试绘制
	if (bShowDebugVisualization)
	{
		DrawDebugVisualization();
	}
}

void AOldManControlStick::StartDragging()
{
	if (!StickHead) return;

	bIsBeingDragged = true;
	bInAutoBack = false;          // 打断自动回正
	SetActorTickEnabled(true);

	// 将当前偏移作为起始点，后续由 UpdateDragPosition 更新目标
	TargetOffset = CurrentOffset;
	MovementAlpha = 1.0f;          // 直接到达当前位置，不产生跳跃
}

void AOldManControlStick::StopDragging()
{
	if (!StickHead) return;

	bIsBeingDragged = false;

	if (bEnableAutoBack)
	{
		// 启动自动回正
		bInAutoBack = true;
		AutoBackStartOffset = CurrentOffset;
		AutoBackTimer = 0.0f;
	}
	else
	{
		// 无自动回正：保持当前位置，停止Tick
		SetActorTickEnabled(false);
	}
}

void AOldManControlStick::UpdateDragPosition(const FVector& WorldPosition)
{
	if (!bIsBeingDragged || !StickHead) return;

	// 将输入点投影到摇杆平面（假设为水平面，Z = InitPos.Z）
	FVector DesiredPos = WorldPosition;
	DesiredPos.Z = InitPos.Z;

	// 计算相对于中心的偏移量
	FVector DesiredOffset = DesiredPos - InitPos;

	// 限制偏移量不超过最大半径
	ClampOffset(DesiredOffset);

	// 可选：应用灵敏度（暂不启用）
	// DesiredOffset *= DragSensitivity;

	// 更新目标偏移
	TargetOffset = DesiredOffset;

	// 重置插值进度，开始平滑移动
	MovementAlpha = 0.0f;
}

void AOldManControlStick::ResetToCenter()
{
	if (!StickHead) return;

	TargetOffset = FVector::ZeroVector;
	MovementAlpha = 0.0f;
	bIsBeingDragged = false;
	bInAutoBack = false;
	SetActorTickEnabled(true);  // 开启Tick以完成平滑移动
}

FVector AOldManControlStick::GetWorldDirection()
{
	return InputDir;
}

FVector AOldManControlStick::GetCurrentOffset() const
{
	return CurrentOffset;
}

void AOldManControlStick::UpdateInputDir()
{
	float Length = CurrentOffset.Size();
	if (Length > DeadZone)
	{
		// 归一化方向，忽略 Z 轴（仅水平面）
		InputDir = CurrentOffset / MaxRadius;
		InputDir.Z = 0.0f;
	}
	else
	{
		InputDir = FVector::ZeroVector;
	}
}

void AOldManControlStick::ClampOffset(FVector& Offset) const
{
	float Length = Offset.Size();
	if (Length > MaxRadius)
	{
		Offset = Offset.GetSafeNormal() * MaxRadius;
	}
}

void AOldManControlStick::DrawDebugVisualization() const
{
	if (!GetWorld()) return;

	// 绘制摇杆活动范围圆（分段绘制线条）
	const int32 Segments = DebugCircleSegments;
	const float Step = 2 * PI / Segments;
	for (int32 i = 0; i < Segments; ++i)
	{
		float Angle1 = i * Step;
		float Angle2 = (i + 1) * Step;
		FVector Point1 = InitPos + FVector(FMath::Cos(Angle1) * MaxRadius, FMath::Sin(Angle1) * MaxRadius, 0);
		FVector Point2 = InitPos + FVector(FMath::Cos(Angle2) * MaxRadius, FMath::Sin(Angle2) * MaxRadius, 0);
		DrawDebugLine(GetWorld(), Point1, Point2, DebugCircleColor, false, -1.0f, 0, 2.0f);
	}

	// 绘制当前摇杆头位置（黄色球体）
	DrawDebugSphere(GetWorld(), StickHead->GetActorLocation(), 10.0f, 8, FColor::Yellow, false, -1.0f, 0);

	// 绘制方向指示线（红色箭头）
	if (!InputDir.IsNearlyZero())
	{
		FVector DirEnd = InitPos + InputDir * MaxRadius;
		DrawDebugDirectionalArrow(GetWorld(), InitPos, DirEnd, 20.0f, FColor::Red, false, -1.0f, 0, 3.0f);
	}
}