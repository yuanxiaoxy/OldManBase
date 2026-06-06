#include "AnimationBall/OldManAnimationBall.h" // 对应头文件
#include "GlobalEventName.h" // 全局事件名（玩家输入开关等）
#include "GlobalTagName.h" // 全局 Tag 名（玩家 Tag）
#include "UIManager/UIManager.h" // UI 管理器（显示/关闭面板）
#include "Components/Image.h" // UMG Image 控件
#include "Components/BoxComponent.h" // UBoxComponent（InteractionBox）
#include "Components/StaticMeshComponent.h" // 静态网格组件
#include "EventManager/MyEventManager.h" // 项目事件管理器
#include "LanguageManager/xyLanguageManager.h" // 多语言管理器
#include "TimerManager.h"

namespace AnimationBallPlayback // 匿名命名空间级：仅本 cpp 可见的播放状态
{
	TWeakObjectPtr<AOldManAnimationBall> ActiveBall; // 当前正在播放的动画球（弱指针防悬空）
}

bool AOldManAnimationBall::IsMediaSetupValid() const // 校验播放所需资源是否配置完整
{
	if (!IsValid(MediaPlayer)) // MediaPlayer 无效则无法播放
	{
		return false; // 校验失败
	}

	if (myType == E_AniBallType::playOnScene) // 场景播放模式额外检查
	{
		if (!IsValid(PlayWall)) // 场景墙 Actor 必须存在
		{
			return false; // 校验失败
		}
		if (!IsCreateOnly && !IsValid(PlayWallMaterial) && !(ShouldFadeIn && IsValid(FadeInMaterial))) // 非仅创建时要有材质或淡入材质
		{
			return false; // 校验失败
		}
	}

	if (myType != E_AniBallType::playAsText && !IsCreateOnly && !IsValid(GetMediaSourceForCurrentLanguage())) // 需要播视频时媒体源必须有效
	{
		return false; // 校验失败
	}

	return true; // 全部检查通过
}

UFileMediaSource* AOldManAnimationBall::GetMediaSourceForCurrentLanguage() const // 根据当前语言返回对应 FileMediaSource
{
	const UxyLanguageManager* LanguageManager = UxyLanguageManager::GetLanguageManager(); // 取语言管理器单例
	if (!LanguageManager) // 管理器不存在时回退
	{
		return FileMediaSource; // 默认中文源
	}

	switch (LanguageManager->GetCurrentLanguage()) // 按当前语言分支
	{
	case ELanguageType::English: // 英文
		return IsValid(FileMediaSourceInEnglish) ? FileMediaSourceInEnglish : FileMediaSource; // 英文源优先，否则回退中文
	case ELanguageType::Chinese: // 中文
	default: // 其它语言走默认
		return FileMediaSource; // 中文源
	}
}

bool AOldManAnimationBall::OpenMediaSourceForCurrentLanguage() // 对 MediaPlayer 打开当前语言媒体
{
	if (!IsValid(MediaPlayer)) // 播放器无效
	{
		return false; // 打开失败
	}

	UFileMediaSource* Source = GetMediaSourceForCurrentLanguage(); // 解析媒体源
	if (!IsValid(Source)) // 源无效
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_当前语言媒体源不存在")); // 打日志
		return false; // 打开失败
	}

	return MediaPlayer->OpenSource(Source); // 异步打开，成功返回 true 仅表示开始尝试
}

bool AOldManAnimationBall::IsActiveAnimationBall() const // 是否为本场景当前“活跃”动画球
{
	return AnimationBallPlayback::ActiveBall.Get() == this; // 与全局弱指针比较
}

void AOldManAnimationBall::HandoffFromPreviousBall() // 新球开始播前：结束上一球
{
	if (!IsCreateOnly)
	{
		if (AOldManAnimationBall* PreviousBall = AnimationBallPlayback::ActiveBall.Get()) // 取上一活跃球
		{
			if (PreviousBall != this && IsValid(PreviousBall)) // 存在且不是本球
			{
				PreviousBall->PlayOverInterruptedByHandoff(); // 收尾上一球，但不 Close 共享 MediaPlayer
			}
		}
		AnimationBallPlayback::ActiveBall = this; // 将本球登记为当前活跃球
	}

}

void AOldManAnimationBall::UnbindMediaDelegates() // 移除本 Actor 在 MediaPlayer 上的动态委托
{
	if (!IsValid(MediaPlayer)) // 播放器无效则无需解绑
	{
		return; // 直接返回
	}

	MediaPlayer->OnMediaOpened.RemoveDynamic(this, &AOldManAnimationBall::OnMediaOpened_ChooseType); // 解绑：打开后按类型
	MediaPlayer->OnEndReached.RemoveDynamic(this, &AOldManAnimationBall::PlayOver); // 解绑：播放到结尾
}

void AOldManAnimationBall::BindPlaybackEndDelegates() // 仅在媒体打开成功后绑定结束回调
{
	if (!IsValid(MediaPlayer) || !IsActiveAnimationBall())
	{
		return;
	}

	MediaPlayer->OnEndReached.RemoveDynamic(this, &AOldManAnimationBall::PlayOver);
	MediaPlayer->OnEndReached.AddDynamic(this, &AOldManAnimationBall::PlayOver);
}

void AOldManAnimationBall::HideIfDisposableTriggered() // 一次性球触发后隐藏自身
{
	if (!Disposable) // Disposable 为 false 表示已消耗一次性
	{
		Print(TEXT("执行死亡")); // 调试输出
		SetActorHiddenInGame(true); // 游戏中隐藏
		SetActorEnableCollision(false); // 关闭碰撞（不能再触发）
		SetActorTickEnabled(false); // 关闭 Tick
	}
}

void AOldManAnimationBall::EnsureMediaSoundComponent() // 创建或更新媒体声音组件
{
	if (IsValid(MediaSoundComponent)) // 已存在则只更新关联
	{
		MediaSoundComponent->SetMediaPlayer(MediaPlayer); // 绑定到当前 MediaPlayer
		return; // 结束
	}

	MediaSoundComponent = NewObject<UMediaSoundComponent>(this, UMediaSoundComponent::StaticClass()); // 新建组件对象
	if (!IsValid(MediaSoundComponent)) // 创建失败
	{
		return; // 结束
	}

	MediaSoundComponent->RegisterComponent(); // 注册到世界以便发声
	MediaSoundComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform); // 挂到根组件
	AddInstanceComponent(MediaSoundComponent); // 加入实例组件列表便于管理
	MediaSoundComponent->SetMediaPlayer(MediaPlayer); // 关联播放器
}

void AOldManAnimationBall::BeginPlay() // Actor 进入关卡时
{
	Super::BeginPlay(); // 调用父类 BeginPlay

	if (!IsValid(FileMediaSource)) // 检查中文媒体源
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体源不存在")); // 警告日志
	}
	if (!IsValid(MediaPlayer)) // 检查播放器
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体播放器组件不存在")); // 警告日志
	}
	if (!IsValid(MediaTexture)) // 检查纹理
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体纹理不存在")); // 警告日志
	}

	if (myType == E_AniBallType::playOnScene) // 场景模式初始化 PlayWall
	{
		if (!IsValid(PlayWall)) // 墙不存在
		{
			UE_LOG(LogTemp, Warning, TEXT("AB_场景中播放的物体不存在")); // 警告日志
		}
		if (!IsValid(PlayWallMaterial)) // 材质未配
		{
			UE_LOG(LogTemp, Warning, TEXT("AB_场景中播放的物体上的材质")); // 警告日志
		}
		if (IsValid(PlayWall) && !PlayWall->IsHidden()) // 墙存在且当前可见
		{
			PlayWall->SetActorHiddenInGame(true); // 开局先隐藏墙
			PlayWall->SetActorEnableCollision(false); // 关闭墙碰撞
			PlayWall->SetActorTickEnabled(false); // 关闭墙 Tick
		}
	}

	if (IsValid(MediaPlayer)) // 播放器有效
	{
		MediaPlayer->Close(); // 清空/关闭可能残留的上次媒体状态
	}
}

void AOldManAnimationBall::EndPlay(const EEndPlayReason::Type EndPlayReason) // Actor 离开关卡/销毁前
{
	if (bMediaPrepared && PlayerInputCancel) // 若在准备态且曾禁用输入
	{
		UMyEventManager::GetEventManager()->TriggerCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), true); // 恢复玩家输入
	}

	UnbindMediaDelegates(); // 解绑所有媒体委托
	bMediaPrepared = false; // 清除准备标记

	if (AnimationBallPlayback::ActiveBall.Get() == this) // 若本球仍是活跃球
	{
		AnimationBallPlayback::ActiveBall.Reset(); // 清空全局活跃引用
	}

	Super::EndPlay(EndPlayReason); // 调用父类 EndPlay
}

bool AOldManAnimationBall::CanStartPlayback() const // 检查是否满足开始播放条件
{
	return Disposable && IsMediaSetupValid() && !bMediaPrepared; // 满足：未消耗、配置有效、未在播放
}

void AOldManAnimationBall::NotifyBlueprintEnterTrigger(AActor* TriggerActor) // 通知蓝图进入触发
{
	OnEnterTrigger(InteractionBox, TriggerActor, nullptr, 0, false, FHitResult()); // 调用父类虚函数触发蓝图事件
}

void AOldManAnimationBall::ClearSharedMediaTextureFrame() // 清掉 MediaTexture 上残留的最后一帧
{
	if (!IsValid(MediaTexture))
	{
		return;
	}

	MediaTexture->SetMediaPlayer(nullptr);

	if (IsValid(MediaPlayer))
	{
		MediaTexture->SetMediaPlayer(MediaPlayer);
	}
}

void AOldManAnimationBall::PrepareSharedMediaForNewClip() // 为新片段准备共享媒体资源
{
	if (!IsValid(MediaPlayer)) // 播放器无效则直接返回
	{
		return;
	}

	if (!IsCreateOnly)
	{
		MediaPlayer->Close(); // 非仅创建模式则关闭现有媒体
		ClearSharedMediaTextureFrame(); // 避免新片段打开前仍显示上一段最后一帧
	}
	else if (IsValid(MediaTexture))
	{
		MediaTexture->SetMediaPlayer(MediaPlayer); // 仅创建模式：不关媒体，只保持关联
	}
}

void AOldManAnimationBall::ApplySceneVideoMaterial() // 延迟将视频材质应用到场景 PlayWall
{
	if (!IsActiveAnimationBall() || !bMediaPrepared || IsCreateOnly)
	{
		return;
	}

	if (!IsValid(PlayWall))
	{
		return;
	}

	UStaticMeshComponent* PlayWallMesh = PlayWall->GetStaticMeshComponent();
	if (!IsValid(PlayWallMesh) || !IsValid(PlayWallMaterial))
	{
		return;
	}

	PlayWallMesh->SetMaterial(0, PlayWallMaterial);
}

void AOldManAnimationBall::ApplyUIVideoBrush() // 将视频材质应用到 UI Image
{
	if (!IsActiveAnimationBall() || !bMediaPrepared || CurUIName.IsNone()) // 非活跃球或未准备好则返回
	{
		return;
	}

	UUserWidget* CurWidget = UUIManager::GetInstance()->GetUI(CurUIName); // 获取当前 UI 面板
	if (!IsValid(CurWidget)) // 面板无效则返回
	{
		return;
	}

	UImage* CurImg = Cast<UImage>(CurWidget->GetWidgetFromName(TEXT("Image_0"))); // 获取 Image 控件
	if (!IsValid(CurImg) || !IsValid(PlayWallMaterial)) // 图片或材质无效则返回
	{
		return;
	}

	CurImg->SetBrushFromMaterial(PlayWallMaterial); // 设置图片为视频材质
	CurImg->SetRenderOpacity(1.f); // 设置完全不透明
}

void AOldManAnimationBall::SetGlobalTime(float Time) // 设置全局时间膨胀
{
	if (UWorld* World = GetWorld()) // 获取世界上下文
	{
		if (AWorldSettings* WorldSettings = World->GetWorldSettings()) // 获取世界设置
		{
			WorldSettings->SetTimeDilation(Time); // 设置时间膨胀比例（0=暂停，1=正常）
		}
	}
}

bool AOldManAnimationBall::StartPlayback(AActor* TriggerActor) // 开始播放入口
{
	if (bMediaPrepared) // 已经在准备或播放中
	{
		UE_LOG(LogTemp, Verbose, TEXT("AB_StartPlayback: 已在播放准备中，忽略重复调用"));
		return false;
	}

	if (!Disposable) // 已经触发过一次性
	{
		UE_LOG(LogTemp, Verbose, TEXT("AB_StartPlayback: 已触发过（Disposable=false）"));
		return false;
	}

	if (!IsMediaSetupValid()) // 配置无效
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_StartPlayback: 媒体/场景配置无效"));
		return false;
	}

	if (IsDisposable) // 如果是消耗性球则标记为已消耗
	{
		Disposable = false;
	}

	HandoffFromPreviousBall(); // 处理上一个球的交接
	PrepareSharedMediaForNewClip(); // 准备共享媒体资源
	BeforePreparation(); // 播放前准备

	MediaPlayer->OnMediaOpened.AddDynamic(this, &AOldManAnimationBall::OnMediaOpened_ChooseType); // 绑定打开回调

	if (!IsCreateOnly) // 非仅创建模式
	{
		OpenMediaSourceForCurrentLanguage(); // 打开对应语言的媒体源
	}
	else // 仅创建模式
	{
		BindPlaybackEndDelegates(); // 直接绑定结束回调
		ChooseType(); // 跳过媒体打开直接执行播放分支
	}

	HideIfDisposableTriggered(); // 处理消耗性隐藏

	NotifyBlueprintEnterTrigger(TriggerActor); // 触发蓝图 OnEnterTrigger：暂停游戏、Niagara、音效等

	return true; // 播放开始成功
}

void AOldManAnimationBall::PlayVideoInUI() // 兼容旧蓝图名的播放接口
{
	StartPlayback(nullptr); // 直接调用主播放函数
}

void AOldManAnimationBall::Interect(FOldManItemInteractData interectData) // 交互触发入口
{
	Super::Interect(interectData); // 调用基类实现
	StartPlayback(interectData.InteractingActor); // 开始播放，传递触发玩家
}

void AOldManAnimationBall::PlayAniInScene() // 激活场景墙并设置材质
{
	UE_LOG(LogTemp, Display, TEXT("AB_scene")); // 日志：场景分支

	if (!IsValid(PlayWall)) // 墙无效
	{
		return; // 退出
	}

	UStaticMeshComponent* PlayWallMesh = PlayWall->GetStaticMeshComponent(); // 取静态网格组件
	if (!IsValid(PlayWallMesh)) // 组件无效
	{
		return; // 退出
	}

	if (PlayWall->IsHidden()) // 若墙当前隐藏
	{
		PlayWall->SetActorHiddenInGame(false); // 显示墙
		PlayWall->SetActorEnableCollision(true); // 开启碰撞（若需要）
		PlayWall->SetActorTickEnabled(true); // 开启 Tick
	}

	if (IsCreateOnly) // 仅创建：只显示淡入墙，不贴视频材质，避免共享纹理残留帧
	{
		if (IsValid(FadeInMaterial))
		{
			PlayWallMesh->SetMaterial(0, FadeInMaterial);
			if (ShouldFadeIn)
			{
				StartFadeIn();
			}
		}
		return;
	}

	if (ShouldFadeIn && IsValid(FadeInMaterial)) // 需要淡入且有淡入材质
	{
		PlayWallMesh->SetMaterial(0, FadeInMaterial); // 槽 0 设为淡入材质
		StartFadeIn(); // 通知蓝图做淡入
	}
	else if (IsValid(PlayWallMaterial)) // 普通播放：延迟贴材质，等新片段首帧就绪
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(DeferredSceneVideoMaterialTimer);
			World->GetTimerManager().SetTimer(
				DeferredSceneVideoMaterialTimer,
				this,
				&AOldManAnimationBall::ApplySceneVideoMaterial,
				0.05f,
				false);
		}
	}
}

void AOldManAnimationBall::StartPlaybackIfNeeded() // 媒体已打开但未播放时补 Play
{
	if (IsValid(MediaPlayer) && !MediaPlayer->IsPlaying()) // 有效且未在播
	{
		MediaPlayer->Play(); // 开始播放
	}
}

void AOldManAnimationBall::OnMediaOpened_ChooseType(FString /*OpenedUrl*/) // OnMediaOpened：按 myType 分发
{
	if (!bMediaPrepared || !IsActiveAnimationBall()) // 未准备或非活跃球则忽略
	{
		return; // 退出
	}

	BindPlaybackEndDelegates(); // 打开成功后再绑 OnEndReached
	StartPlaybackIfNeeded(); // 确保开始播放
	ChooseType(); // 场景 / UI / 文本 分支
}

void AOldManAnimationBall::PlayAniInUI() // 根据语言与选项打开对应 UI
{
	UE_LOG(LogTemp, Display, TEXT("AB_UI")); // 日志：UI 分支

	UAniMationBallDatas* Datas = NewObject<UAniMationBallDatas>(this); // 构造传给 UI 的数据对象
	const UxyLanguageManager* LanguageManager = UxyLanguageManager::GetLanguageManager(); // 语言管理器
	const ELanguageType CurrentLanguage = LanguageManager // 当前语言
		? LanguageManager->GetCurrentLanguage() // 有管理器则查询
		: ELanguageType::Chinese; // 否则默认中文
	Datas->MediaPlayer = MediaPlayer; // 供 UI 内控制播放/跳过
	Datas->AnimationBall = this; // 供 UI 回调本球
	Datas->IsPauseGame = IsPauseGame;// 供 UI 确定是否暂停游戏

	if (IsOpenSkip) // 需要跳过按钮的面板
	{
		if (IsTouMing) // 透明背景款
		{
			switch (CurrentLanguage) // 按语言选 UI 名
			{
			case ELanguageType::English: // 英文透明+按钮
				UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanelWithButtonInEnglish", Datas); // 显示 UI
				CurUIName = "AnimationPlayPanelWithButtonInEnglish"; // 记录名称供关闭
				break; // 结束 case
			case ELanguageType::Chinese: // 中文透明+按钮
			default: // 默认中文
				UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanelWithButton", Datas); // 显示 UI
				CurUIName = "AnimationPlayPanelWithButton"; // 记录名称
				break; // 结束 case
			}
		}
		else // 黑色背景款
		{
			switch (CurrentLanguage) // 按语言选 UI 名
			{
			case ELanguageType::English: // 英文黑底+按钮
				UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanelWithButtonInEnglishWithBlack", Datas); // 显示 UI
				CurUIName = "AnimationPlayPanelWithButtonInEnglishWithBlack"; // 记录名称
				break; // 结束 case
			case ELanguageType::Chinese: // 中文黑底+按钮
			default: // 默认中文
				UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanelWithButtonWithBlack", Datas); // 显示 UI
				CurUIName = "AnimationPlayPanelWithButtonWithBlack"; // 记录名称
				break; // 结束 case
			}
		}
	}
	//else if (IsTouMing) // 无跳过、透明
	//{
	//	UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanel", Datas); // 显示简单透明面板
	//	CurUIName = "AnimationPlayPanel"; // 记录名称
	//}
	else // 无跳过、默认无跳过为黑底，改为透明底的话，将上述代码回复，自己配一个对应名称的UIPanel
	{
		UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanelWithBlack", Datas); // 显示黑底面板
		CurUIName = "AnimationPlayPanelWithBlack"; // 记录名称
	}

	if (UUserWidget* CurWidget = UUIManager::GetInstance()->GetUI(CurUIName))
	{
		if (UImage* CurImg = Cast<UImage>(CurWidget->GetWidgetFromName(TEXT("Image_0"))))
		{
			CurImg->SetRenderOpacity(0.f);
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DeferredUIVideoBrushTimer);
		World->GetTimerManager().SetTimer(
			DeferredUIVideoBrushTimer,
			this,
			&AOldManAnimationBall::ApplyUIVideoBrush,
			0.05f,
			false);
	}
}

void AOldManAnimationBall::ChooseType() // 按 myType 调用具体播放实现
{
	switch (myType) // 分支
	{
	case E_AniBallType::playOnScene: // 场景
		PlayAniInScene(); // 显示墙
		break; // 结束 case
	case E_AniBallType::playOnUI: // UI
		PlayAniInUI(); // 开面板
		break; // 结束 case
	case E_AniBallType::playAsText: // 文本
		PlayText(); // 文本逻辑
		break; // 结束 case
	default: // 未配置类型
		UE_LOG(LogTemp, Warning, TEXT("AB_你不用，还不删，留着过年呢")); // 警告
		if (GEngine) // 编辑器/游戏有 GEngine
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AB_你不用，还不删，留着过年呢")); // 屏幕提示
		}
		break; // 结束 case
	}
}

void AOldManAnimationBall::PlayText() // 文本模式（待实现）
{
	UE_LOG(LogTemp, Display, TEXT("AB_text")); // 日志占位
}

void AOldManAnimationBall::CleanupPlayback(bool bCloseMedia, bool bRestorePlayerInput, bool bDestroyActor) // 统一清理入口
{
	if (!bMediaPrepared) // 未处于播放状态则直接返回
	{
		return;
	}

	if (!IsActiveAnimationBall()) // 非活跃球则只清理自身状态
	{
		bMediaPrepared = false; // 重置标记
		UnbindMediaDelegates(); // 解绑委托
		return;
	}

	bMediaPrepared = false; // 重置播放准备标记
	UE_LOG(LogTemp, Display, TEXT("AB_Over")); // 日志：播放结束

	UnbindMediaDelegates(); // 先解绑，避免 Close/Open 时误回调到本球

	if (UWorld* World = GetWorld()) // 获取世界
	{
		World->GetTimerManager().ClearTimer(DeferredUIVideoBrushTimer); // 清除延迟刷 UI 材质定时器
		World->GetTimerManager().ClearTimer(DeferredSceneVideoMaterialTimer); // 清除延迟刷场景墙材质定时器
	}

	VideoPlayCompelete(); // 调用蓝图播放完成事件

	if (bRestorePlayerInput && PlayerInputCancel) // 需要恢复输入且配置了禁用输入
	{
		UMyEventManager::GetEventManager()->TriggerCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), true); // 恢复玩家输入
	}

	if (myType == E_AniBallType::playOnScene && IsValid(PlayWall) && !PlayWall->IsHidden() && !IsCreateOnly) // 场景模式且墙可见且非仅创建
	{
		PlayWall->SetActorHiddenInGame(true); // 隐藏播放墙
		PlayWall->SetActorEnableCollision(false); // 关闭碰撞
		PlayWall->SetActorTickEnabled(false); // 关闭 Tick
	}

	if (myType == E_AniBallType::playOnUI && !CurUIName.IsNone()) // UI 模式且有打开的面板
	{
		UUIManager::GetInstance()->CloseUI(CurUIName, true, true); // 关闭并销毁 UI 面板
		if (IsPauseGame) SetGlobalTime(1); // 如果暂停过游戏则恢复
		CurUIName = NAME_None; // 重置面板名称
	}

	if (bCloseMedia && IsValid(MediaPlayer)) // 需要关闭媒体且播放器有效
	{
		MediaPlayer->Close(); // 关闭并清理媒体资源
	}

	if (AnimationBallPlayback::ActiveBall.Get() == this) // 自身是活跃球则清除全局引用
	{
		AnimationBallPlayback::ActiveBall.Reset();
	}

	if (bDestroyActor) // 需要销毁 Actor
	{
		Destroy(); // 销毁本 Actor
	}
}

void AOldManAnimationBall::PlayOver() // 自然结束或跳过：完整清理并销毁
{
	CleanupPlayback(true, true, true); // 关闭媒体、恢复输入、销毁 Actor
}

void AOldManAnimationBall::PlayOverInterruptedByHandoff() // 被新球顶替：保留共享资源
{
	CleanupPlayback(false, false, true); // 不关媒体、不恢复输入、销毁 Actor
}

void AOldManAnimationBall::BeforePreparation() // 每次开始播放前的准备
{
	if (!IsValid(MediaPlayer)) // 无播放器则直接返回
	{
		return;
	}

	UnbindMediaDelegates(); // 先清旧绑定，避免重复

	MediaPlayer->SetLooping(Loop); // 设置是否循环
	bMediaPrepared = true; // 标记已进入准备态

	if (PlayerInputCancel) // 需要禁用玩家输入
	{
		UMyEventManager::GetEventManager()->TriggerCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), false); // 发送禁用事件
	}

	EnsureMediaSoundComponent(); // 确保有声音组件

	if (IsValid(MediaTexture) && IsValid(MediaPlayer)) // 纹理和播放器都有效
	{
		MediaTexture->SetMediaPlayer(MediaPlayer); // 关联纹理和播放器
	}
}

void AOldManAnimationBall::Print(FString text) // 屏幕打印调试信息
{
	if (GEngine) // GEngine 可用（PIE/Game）
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AB_") + text); // 黄字 2 秒
	}
}

void AOldManAnimationBall::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, // 玩家进入触发框回调
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !OtherActor->ActorHasTag(UGlobalTagName::Tag_Player)) // 不是玩家则直接返回
	{
		return;
	}

	StartPlayback(OtherActor); // 开始播放，传递触发玩家
}
/*
				   _ooOoo_
				  o8888888o
				  88" . "88
				  (| -_- |)
				  O\  =  /O
			   ____/`---'\____
			 .'  \\|     |//  `.
			/  \\|||  :  |||//  \
		   /  _||||| -:- |||||-  \
		   |   | \\\  -  /// |   |
		   | \_|  ''\---/''  |   |
		   \  .-\__  `-`  ___/-. /
		 ___`. .'  /--.--\  `. . __
	  ."" '<  `.___\_<|>_/___.'  >'"".
	 | | :  `- \`.;`\ _ /`;.`/ - ` : | |
	 \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
				   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			佛祖保佑       永无BUG
*/
