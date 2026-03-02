// OldManHead.h
// 定义老人头类，是 Boss 战的核心实体。负责旋转控制、随机晃动、进度管理、阶段切换。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OldManHead.generated.h"

// 前向声明
class AOldManMobileCamera;
class AOldManControlStick;

/** 攻击类型枚举 */
UENUM(BlueprintType)
enum class EOldManAttactType : uint8
{
	Halo,  // 光圈攻击
	Beam   // 光束攻击
};

/** 阶段信息结构体，每个阶段包含成功旋转范围、阈值、攻击间隔等 */
USTRUCT(BlueprintType)
struct FPhaseInfo
{
	GENERATED_USTRUCT_BODY()

	/** 成功旋转范围偏移值（相对于初始旋转），仅比较 Yaw 和 Pitch */
	UPROPERTY(EditAnywhere, Category = "SuccessRange")
	FRotator RotRangeOffset;

	/** 进入下一阶段所需的进度值 */
	UPROPERTY(EditAnywhere, Category = "Progress")
	float PhaseThreshold = 20;

	/** 该阶段下 Boss 的攻击间隔（秒） */
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackCooldown;

	/** 该阶段的攻击类型 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	EOldManAttactType AttactType;

	/** Beam子弹持续时间 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	float ContinueTime = 1;

	/** 对应攻击类型的进度值（备用） */
	UPROPERTY(EditAnywhere, Category = "Attack")
	float Progress;

	/** 子弹移动速度 */
	UPROPERTY(EditAnywhere, Category = "Attack")
	float MoveSpeed = 100;
};

/** 阶段枚举 */
UENUM(BlueprintType)
enum class EOldManHeadType : uint8
{
	one,   // 阶段1
	two,   // 阶段2
	three, // 阶段3
	end    // 结束
};

/** 阶段变化事件委托：参数为旧阶段和新阶段（int32 表示枚举值） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhaseChanged, int32, OldPhase, int32, NewPhase);

/**
 * 老人头类。
 * - 接受摇杆输入，转换为 Yaw 和 Pitch 旋转。
 * - 随机晃动影响旋转。
 * - 管理当前进度和阶段，触发阶段切换事件。
 * - 作为进度条管理器，持有 Boss 和摇杆引用，控制全局光束标志。
 */
UCLASS()
class OLDMAN_API AOldManHead : public AActor
{
	GENERATED_BODY()

public:
	AOldManHead();

	// ===== 旋转限制 =====
	/** Yaw 轴最大偏移（度） */
	UPROPERTY(EditAnywhere, Category = "Rotation Limits", meta = (ClampMin = 0.0f, ClampMax = 90.0f))
	float YawMax = 90.0f;

	/** Pitch 轴最大偏移（度） */
	UPROPERTY(EditAnywhere, Category = "Rotation Limits", meta = (ClampMin = 0.0f, ClampMax = 45.0f))
	float PitchMax = 30.0f;

	// ===== 进度系统 =====
	/** 当前进度值 */
	UPROPERTY(EditAnywhere, Category = "Progress")
	float CurProgress = 0;

	/** 当前阶段 */
	UPROPERTY(EditAnywhere, Category = "Progress")
	EOldManHeadType CurPhaseTYpe = EOldManHeadType::one;

	/** 阶段信息数组（索引0=阶段1，1=阶段2，2=阶段3） */
	UPROPERTY(EditAnywhere, Category = "Progress")
	TArray<FPhaseInfo> PhaseInfos;

	/** 当前阶段对应的数组索引 */
	UPROPERTY(EditAnywhere, Category = "Progress")
	int Index;

	// ===== 摇杆控制 =====
	/** 摇杆旋转速度（度/秒） */
	UPROPERTY(EditAnywhere, Category = "ControlStick")
	float StickRotateSpeed = 90.0f;

	/** 是否允许摇杆控制旋转 */
	UPROPERTY(EditAnywhere, Category = "ControlStick")
	bool StickCanMove = false;

	// ===== 随机晃动 =====
	/** 晃动幅度 */
	UPROPERTY(EditAnywhere, Category = "RandomShake")
	float ShakeAmplitude = 1.0f;

	/** 晃动频率 */
	UPROPERTY(EditAnywhere, Category = "RandomShake")
	float ShakeFrequency = 1.0f;

	/** Yaw 轴晃动倍率 */
	UPROPERTY(EditAnywhere, Category = "RandomShake", meta = (ClampMin = 0.0f, ClampMax = 10.0f))
	float YawRate = 1.0f;

	/** Pitch 轴晃动倍率 */
	UPROPERTY(EditAnywhere, Category = "RandomShake", meta = (ClampMin = 0.0f, ClampMax = 10.0f))
	float PitchRate = 1.0f;

	/** 晃动计时器，用于生成 Perlin 噪声 */
	UPROPERTY(EditAnywhere, Category = "Else")
	float ShakeTimer = 0;

	// ===== 管理器职责（事件、引用等）=====
	/** 阶段变化事件（广播给 Boss 等） */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPhaseChanged OnPhaseChanged;

	/** UI 文本组件，显示当前阶段 */
	UPROPERTY(EditAnywhere, Category = "UI")
	class UTextRenderComponent* PhaseText;

	/** 引用 Boss（移动相机） */
	UPROPERTY(EditAnywhere, Category = "References")
	AOldManMobileCamera* MoblieCamera;

	/** 引用摇杆 */
	UPROPERTY(EditAnywhere, Category = "References")
	AOldManControlStick* ControlStick;

	/** 全局光束标志，用于阶段切换限制（阶段2不能在光束中切换） */
	bool bIsBeamActive;

	// ===== 方法 =====
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	/** 应用摇杆输入（方向向量），映射到 Yaw 和 Pitch 偏移 */
	UFUNCTION(BlueprintCallable, Category = "ControlStick")
	void ApplyInput(const FVector& Direction);

	/** 随机晃动（仅影响 Yaw 和 Pitch） */
	void RandomShake(float DeltaTime);

	/** 播放眨眼动画（增加进度） */
	void Blink(float progress);

	/** 检测当前旋转是否在指定阶段的成功范围内 */
	bool IsInSuccessRange(const FRotator& Rotation, EOldManHeadType curType);

	/** 攻击命中回调，可能立即或延迟增加进度 */
	void OnAttackHit(float addProgress, float delayTime = 0);

	/** 增加进度，检查是否达到阶段阈值 */
	void AddProgress(float addProgress);

	/** 减少进度（备用） */
	void SubtractProgress(float progress);

	/** 尝试切换到新阶段，检查条件并触发事件 */
	void ChangePhase(EOldManHeadType newPhase);

	/** 辅助函数：比较两个旋转的 Yaw 和 Pitch 绝对值是否小于等于 */
	bool FRotatorLessAbs(const FRotator& left, const FRotator& right);

	/** 根据当前阶段更新索引 */
	void UpdateIndex(EOldManHeadType curType);

	/** 设置光束状态（由光束攻击调用） */
	void SetBeamActive(bool bActive) { bIsBeamActive = bActive; }

	/** 是否处于光束攻击中 */
	bool IsInBeamAttack() const { return bIsBeamActive; }

private:
	FRotator InitRot;               // 初始旋转
	FRotator ShakeOffsetRot;        // 晃动产生的旋转偏移
	FRotator PlayerInputRot;        // 摇杆输入产生的旋转偏移
	FRotator OffsetRot;             // 总旋转偏移（用于应用旋转）

	float DelayTime;                 // 延迟判定时间（用于持续攻击如光束）
	float Timer;                     // 延迟计时器
	bool IsOpenRange;                // 是否开启持续范围检测
	float TempAddProgress;           // 临时进度值（用于持续判定）
	bool CanChangePhase;             // 是否允许切换阶段（到达 end 后禁止）
};