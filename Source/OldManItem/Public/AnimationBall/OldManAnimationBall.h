// Fill out your copyright notice in the Description page of Project Settings. // 版权声明占位（引擎模板默认行）

#pragma once // 头文件保护：防止重复包含

#include "CoreMinimal.h" // UE 核心最小头文件
#include "ItemBase/OldManInterectItemBase.h" // 交互物品基类
#include "Engine/StaticMeshActor.h" // 静态网格 Actor（场景播放墙）
#include "FileMediaSource.h" // 文件媒体源
#include "MediaPlayer.h" // 媒体播放器
#include "MediaTexture.h" // 媒体纹理
#include "MediaSoundComponent.h" // 媒体音频组件
#include "OldManAnimationBall.generated.h" // UHT 生成代码（必须放在最后一个 include）

class AOldManAnimationBall; // 前向声明：供 UAniMationBallDatas 引用

/** 动画球播放类型 */
UENUM(BlueprintType) // 暴露给蓝图使用的枚举
enum class E_AniBallType : uint8 // 枚举底层类型 uint8
{
	None UMETA(DisplayName = "None"), // 无类型
	playOnScene UMETA(DisplayName = "Play On Scene"), // 在场景中播放
	playOnUI UMETA(DisplayName = "Play On UI"), // 在 UI 上播放
	playAsText UMETA(DisplayName = "Play As Text") // 以文本/对话形式播放
};

/** 传给 UI 面板的播放数据 */
UCLASS(BlueprintType) // 可在蓝图里创建/使用的 UObject
class OLDMANITEM_API UAniMationBallDatas : public UObject // 模块导出 + 继承 UObject
{
	GENERATED_BODY() // UHT 生成反射体

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) // 编辑器可改、蓝图可读写
	UMediaPlayer* MediaPlayer; // UI 跳过按钮等需要操作的播放器指针

	UPROPERTY(EditAnywhere, BlueprintReadWrite) // 编辑器可改、蓝图可读写
	AOldManAnimationBall* AnimationBall; // 当前触发的动画球 Actor 指针
};

/** 动画球 Actor：玩家触发后按类型播视频/显 UI */
UCLASS() // 注册为 UE 类
class OLDMANITEM_API AOldManAnimationBall : public AOldManInterectItemBase // 继承可交互物品基类
{
	GENERATED_BODY() // UHT 生成反射体

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") // 媒体分类
	UFileMediaSource* FileMediaSource; // 中文（默认）视频文件源

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") // 媒体分类
	UFileMediaSource* FileMediaSourceInEnglish; // 英文视频文件源

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") // 媒体分类
	UMediaPlayer* MediaPlayer; // 媒体播放器（多球可能共用同一资源）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") // 媒体分类
	UMediaTexture* MediaTexture; // 媒体纹理（材质显示视频用）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") // 媒体分类
	UMaterial* PlayWallMaterial; // 场景墙/UI 图片使用的材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType") // 动画球类型分类
	E_AniBallType myType = E_AniBallType::playOnScene; // 播放模式，默认场景播放

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (ClampMin = 0.0f)) // 时长下限 0
	float CountdownTime = 0.0f; // 视频时长（计时器逻辑预留）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (EditCondition = "myType == E_AniBallType::playOnScene")) // 仅场景模式显示
	bool IsCreateOnly = false; // 是否只生成场景物体、不打开媒体

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (EditCondition = "myType == E_AniBallType::playOnUI")) // 仅 UI 模式显示
	bool IsOpenSkip = false; // UI 是否显示跳过按钮

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (ClampMin = 0.0f)) // 起始时间下限 0
	float BeginTime = 0.0f; // 从第几秒开始播放（预留）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType") // 动画球类型分类
	bool Loop = false; // 是否循环播放

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType") // 动画球类型分类
	bool IsDisposable = true; // 是否一次性（触发后隐藏自身）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType") // 动画球类型分类
	bool PlayerInputCancel = true; // 播放期间是否禁用玩家输入

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType") // 动画球类型分类
	bool IsTouMing = true; // UI 是否使用透明背景面板

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", // 动画球类型分类
		meta = (EditCondition = "myType == E_AniBallType::playAsText")) // 仅文本模式显示
	bool IsAutoText = true; // 对话框是否自动播放（预留）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", // 动画球类型分类
		meta = (EditCondition = "myType == E_AniBallType::playOnScene")) // 仅场景模式显示
	AStaticMeshActor* PlayWall; // 场景中用于显示视频的墙体 Actor

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallFade", meta = (EditCondition = "ShouldFadeIn == true")) // 开启淡入时显示
	UMaterialInstance* FadeInMaterial; // 淡入用材质实例

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallFade") // 淡入分类
	bool ShouldFadeIn = true; // 是否使用淡入效果

	UFUNCTION(BlueprintImplementableEvent) // 蓝图可实现事件
	void StartFadeIn(); // 开始淡入（蓝图实现具体效果）

	UFUNCTION(BlueprintImplementableEvent) // 蓝图可实现事件
	void VideoPlayCompelete(); // 视频播放完毕（蓝图里做收尾）

	/** 唯一播放入口：碰撞与外部调用均走此函数，按 myType 分支；会触发蓝图 OnEnterTrigger（暂停/特效/音效） */
	UFUNCTION(BlueprintCallable, Category = "AnimationBall", meta = (AdvancedDisplay = "TriggerActor"))
	bool StartPlayback(AActor* TriggerActor = nullptr);

	/** 兼容旧蓝图名，内部等同 StartPlayback */
	UFUNCTION(BlueprintCallable, Category = "AnimationBall", meta = (DeprecatedFunction, DeprecationMessage = "请改用 StartPlayback"))
	void PlayVideoInUI();

	virtual void Interect(FOldManItemInteractData interectData) override;

private:
	bool Disposable = true; // 运行时是否还可触发（一次性用完为 false）

	bool bMediaPrepared = false; // 是否已进入播放准备（绑定了委托等）

	FName CurUIName; // 当前打开的 UI 面板名称

	UPROPERTY() // GC 追踪
	UMediaSoundComponent* MediaSoundComponent = nullptr; // 动态创建的媒体声音组件

	UFUNCTION() // 动态委托要求 UFUNCTION
	void PlayAniInScene(); // 在场景中显示 PlayWall 并设材质

	UFUNCTION() // 动态委托要求 UFUNCTION
	void PlayAniInUI(); // 打开 UI 面板并设置 Image 材质

	UFUNCTION() // 动态委托要求 UFUNCTION
	void ChooseType(); // 按 myType 分发到场景/UI/文本

	UFUNCTION() // 动态委托要求 UFUNCTION
	void OnMediaOpened_ChooseType(FString OpenedUrl); // 媒体打开成功：按类型分支

	void StartPlaybackIfNeeded(); // 若未在播放则调用 Play()

	bool CanStartPlayback() const; // 是否满足开始播放条件

	void PlayText(); // 文本模式逻辑（当前仅占位日志）

	UFUNCTION(BlueprintCallable) // 蓝图可调用
	void PlayOver(); // 播放结束：清理、关 UI、销毁

	void BeforePreparation(); // 播放前：禁输入、建音频组件

	void HandoffFromPreviousBall(); // 新球接管：先让上一个球收尾（不关共享 MediaPlayer）

	void PlayOverInterruptedByHandoff(); // 被新球顶替时的收尾（不 Close 媒体、不恢复输入）

	void BindPlaybackEndDelegates(); // 媒体成功打开后再绑定 OnEndReached

	void CleanupPlayback(bool bCloseMedia, bool bRestorePlayerInput, bool bDestroyActor); // 统一清理入口

	bool IsActiveAnimationBall() const; // 当前是否是全局唯一活跃动画球

	void Print(FString text); // 屏幕调试输出

	bool IsMediaSetupValid() const; // 检查媒体/场景/UI 配置是否齐全

	UFileMediaSource* GetMediaSourceForCurrentLanguage() const; // 按语言取 FileMediaSource

	bool OpenMediaSourceForCurrentLanguage(); // 打开当前语言媒体源

	void UnbindMediaDelegates(); // 解除本 Actor 在 MediaPlayer 上的动态绑定

	void HideIfDisposableTriggered(); // 一次性触发后隐藏碰撞与显示

	void EnsureMediaSoundComponent(); // 确保存在并关联 MediaSoundComponent

	void NotifyBlueprintEnterTrigger(AActor* TriggerActor); // 调用蓝图 OnEnterTrigger（暂停/特效/音效）

	void PrepareSharedMediaForNewClip(); // 新开媒体前清空共享播放器/纹理残留帧

	void ApplyUIVideoBrush(); // 将视频材质赋给 UI Image（延迟一帧避免残影）

	FTimerHandle DeferredUIVideoBrushTimer; // 延迟刷 UI 材质计时器

protected:
	virtual void BeginPlay() override; // 关卡开始时初始化

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override; // Actor 结束时清理

	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override; // 玩家进入触发框
};
