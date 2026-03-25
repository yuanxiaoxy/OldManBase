// OldManMobileCamera.h
// 定义 Boss 类（移动相机）。无实体，负责按阶段发射光圈或光束攻击。
// 攻击流程：倒计时结束 → 发射一颗子弹 → 等待子弹命中/销毁 → 重新开始倒计时。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OldManHead.h"
#include "MonoManager/MonoManager.h"
#include "OldManMobileCamera.generated.h"

/**
 * Boss 类（移动相机）。
 * 根据当前阶段周期性地发射攻击（光圈或光束）。
 * 每次只发射一颗子弹，并等待其销毁后才开始下一次倒计时。
 * 使用 MonoManager 管理定时器。
 */
UCLASS()
class OLDMAN_API AOldManMobileCamera : public AActor
{
	GENERATED_BODY()

public:
	AOldManMobileCamera();

	/** 老人头引用 */
	UPROPERTY(EditAnywhere, Category = "TakeAPhoto")
	AOldManHead* OldManHead;

	/** 光圈攻击类（蓝图或C++类） */
	UPROPERTY(EditAnywhere, Category = "TakeAPhoto")
	TSubclassOf<class AOldManHaloAttack> HaloClass;

	/** 光束攻击类（蓝图或C++类） */
	UPROPERTY(EditAnywhere, Category = "TakeAPhoto")
	TSubclassOf<class AOldManBeamAttack> BeamClass;

	/** 光束持续时间（秒） */
	UPROPERTY(EditAnywhere, Category = "TakeAPhoto")
	float ContinueTime = 1.5f;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** 攻击入口，由定时器触发 */
	void Attack();

	/** 发射光圈攻击 */
	void FireHalo();

	/** 发射光束攻击 */
	void FireBeam();

	/** 预攻击逻辑（未实现） */
	void ReadyAttack();

	/** 由老人头调用的阶段变化通知 */
	void OnPhaseChanged(int32 NewPhase);

private:
	/** 定时器管理器（MonoManager） */
	UMonoManager* MonoManager;

	/** 当前阶段（与老人头同步） */
	int32 CurrentPhase;

	/** 是否正在等待当前子弹命中/销毁 */
	bool bIsWaitingForHit;

	/** 当前活动的子弹（用于监听销毁） */
	UPROPERTY()
	AActor* CurrentBullet;

	/** 子弹销毁时的回调，用于重置等待状态 */
	UFUNCTION()
	void OnBulletDestroyed(AActor* DestroyedActor);

	/** 开始倒计时（单次定时器，使用 MonoManager 的 SetInterval 但实际是循环，通过状态控制发射） */
	void StartCooldown();
};