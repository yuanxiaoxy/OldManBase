// OldManControlStick.h
// 定义可拖拽摇杆类。玩家可通过鼠标/触摸拖动摇杆头，输出归一化的方向向量给老人头。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boss/OldManHead.h"
#include "OldManControlStick.generated.h"

/**
 * 可拖拽摇杆。
 * 摇杆头（StickHead）被约束在以初始位置为中心的圆形区域内。
 * 提供 InputDir 归一化方向向量供老人头使用。
 * 支持平滑移动、自动回正、调试可视化。
 */
UCLASS()
class OLDMAN_API AOldManControlStick : public AActor
{
	GENERATED_BODY()

public:
	AOldManControlStick();

	// ===== 原有成员 =====
	/** 摇杆头（可视部分），通常是静态网格体，用于拖拽交互 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ControlStick")
	AActor* StickHead;

	/** 目标老人头（可选，仅用于关联） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ControlStick")
	AOldManHead* TargetHead;

	// ===== 拖拽参数 =====
	/** 摇杆活动半径（单位） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = (ClampMin = "1.0"))
	float MaxRadius = 100.0f;

	/** 拖拽灵敏度（暂未使用，保留） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = (ClampMin = "0.1"))
	float DragSensitivity = 1.0f;

	/** 最大移动速度（每帧可移动的最大距离），防止跳跃 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = (ClampMin = "0.1"))
	float MaxDragSpeed = 20.0f;

	/** 死区：偏移量小于此值时输入视为零，防止微小漂移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = (ClampMin = "0.0"))
	float DeadZone = 5.0f;

	/** 输入平滑因子（0~1），越大越平滑（暂未使用） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SmoothingFactor = 0.8f;

	// ===== 自动回正 =====
	/** 是否启用自动回正（松开后自动回到中心） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoBack")
	bool bEnableAutoBack = true;

	/** 自动回正速度（单位/秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoBack", meta = (ClampMin = "0.1"))
	float AutoBackSpeed = 200.0f;

	// ===== 调试可视化 =====
	/** 是否在运行时绘制调试图形（圆、方向箭头等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugVisualization = true;

	/** 绘制圆的分段数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	float DebugCircleSegments = 32;

	/** 调试圆的颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	FColor DebugCircleColor = FColor::Green;

	// ===== 运行时状态 =====
	/** 当前归一化的方向向量（供外部读取，例如老人头） */
	UPROPERTY(BlueprintReadOnly, Category = "ControlStick")
	FVector InputDir;

	/** 是否正在被拖拽 */
	UPROPERTY(BlueprintReadOnly, Category = "ControlStick")
	bool bIsBeingDragged;

	// ===== 公共方法 =====
	/** 开始拖拽（由交互系统调用） */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void StartDragging();

	/** 结束拖拽（由交互系统调用） */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void StopDragging();

	/** 更新拖拽位置（每帧由交互系统调用，传入世界空间的目标点，例如鼠标射线与平面的交点） */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void UpdateDragPosition(const FVector& WorldPosition);

	/** 获取当前摇杆头的世界位置偏移量（相对于初始位置） */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	FVector GetCurrentOffset() const;

	/** 重置摇杆到初始位置 */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void ResetToCenter();

	/** 供老人头调用的接口，返回当前归一化方向向量 */
	UFUNCTION(BlueprintCallable, Category = "Drag")
	FVector GetWorldDirection();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	/** 初始位置（摇杆中心） */
	FVector InitPos;

	/** 目标偏移量（世界空间），由 UpdateDragPosition 设置 */
	FVector TargetOffset;

	/** 当前实际偏移量（平滑过渡） */
	FVector CurrentOffset;

	/** 平滑插值进度（0~1），用于从当前位置平滑移动到目标位置 */
	float MovementAlpha;

	// 自动回正相关
	bool bInAutoBack;               // 是否正在自动回正
	FVector AutoBackStartOffset;    // 开始回正时的偏移量
	float AutoBackTimer;             // 回正计时器（0~1）

	/** 内部辅助函数：根据 CurrentOffset 更新 InputDir */
	void UpdateInputDir();

	/** 将偏移量限制在 MaxRadius 内 */
	void ClampOffset(FVector& Offset) const;

	/** 绘制调试图形（活动圆、当前位置、方向箭头） */
	void DrawDebugVisualization() const;
};