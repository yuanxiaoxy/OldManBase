#include "AnimationBall/OldManAnimationBall.h"
#include "GlobalEventName.h"
#include "UIManager/UIManager.h"

//初始化
void AOldManAnimationBall::BeginPlay()
{
	Super::BeginPlay();

	//检测媒体源是否存在
	if (FileMediaSource == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体源不存在"));
	}
	//检测媒体播放器组件是否存在
	if (MediaPlayer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体播放器组件不存在"));
	}
	//检测媒体纹理是否存在
	if (MediaTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体纹理不存在"));
	}

	//场景中播放的物体
	if (myType == E_AniBallType::playOnScene)
	{
		//检测场景中播放的物体是否存在
		if (!PlayWall)
		{
			UE_LOG(LogTemp, Warning, TEXT("AB_场景中播放的物体不存在"));
		}
		//检测材质是否存在
		if (PlayWallMaterial == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AB_场景中播放的物体上的材质"));
		}
		//失活场景物体
		if (!PlayWall->IsHidden())
		{
			PlayWall->SetActorHiddenInGame(true);
			PlayWall->SetActorEnableCollision(false);
			PlayWall->SetActorTickEnabled(false);
		}
	}
	//MediaPlayer->OpenSource(FileMediaSource);
}

//在场景中播放
void AOldManAnimationBall::PlayAniInScene()
{
	UE_LOG(LogTemp, Display, TEXT("AB_scene"));
	//激活场景物体
	if (PlayWall->IsHidden())
	{
		PlayWall->SetActorHiddenInGame(false);
		PlayWall->SetActorEnableCollision(true);
		PlayWall->SetActorTickEnabled(true);
	}
	if (!IsCreateOnly)
	{
		//为场景物体添加材质
		PlayWall->GetStaticMeshComponent()->SetMaterial(0, PlayWallMaterial);
		//打开媒体源
		MediaPlayer->OpenSource(FileMediaSource);
	}

	if (ShouldFadeIn && FadeInMaterial)
	{
		PlayWall->GetStaticMeshComponent()->SetMaterial(0, FadeInMaterial);
		StartFadeIn();
	}
}

//在UI界面上播放
void AOldManAnimationBall::PlayAniInUI()
{
	UE_LOG(LogTemp, Display, TEXT("AB_UI"));
	UUIManager::GetInstance()->ShowUIByName("AnimationPlayPanel", nullptr);
	MediaPlayer->OpenSource(FileMediaSource);
}

//对话框
void AOldManAnimationBall::PlayText()
{
	UE_LOG(LogTemp, Display, TEXT("AB_text"));
}

//播放完毕
void AOldManAnimationBall::PlayOver()
{
	UE_LOG(LogTemp, Display, TEXT("AB_Over"));
	//终止播放
	MediaPlayer->Close();
	//恢复玩家输入
	if (PlayerInputCancel)
	{
		//Player->SetPlayerInput(true);
		UMyEventManager::GetEventManager()->TriggerCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(), true);

	}
	//若是场景物体播放模式 失活PlayWall
	if (myType == E_AniBallType::playOnScene)
	{
		if (!PlayWall->IsHidden())
		{
			PlayWall->SetActorHiddenInGame(true);
			PlayWall->SetActorEnableCollision(false);
			PlayWall->SetActorTickEnabled(false);
		}
	}
	//若是UI物体，关闭UI
	if (myType == E_AniBallType::playOnUI)
	{
		UUIManager::GetInstance()->CloseUI("AnimationPlayPanel");
	}
	//如果是一次性的 销毁自己
	if (!Disposable)
	{
		Print("执行死亡");
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		this->SetActorTickEnabled(false);
		//this->Destroy();
	}
}

//播放前准备
void AOldManAnimationBall::BeforePreparation()
{
	//启用计时器
	//if (CountdownTime > 0)
	//{
	//	UMonoManager::GetInstance()->SetTimeout(CountdownTime - BeginTime, this, &AOldManAnimationBall::PlayOver);
	//}
	//绑定回调
	MediaPlayer->OnEndReached.AddDynamic(this, &AOldManAnimationBall::PlayOver);
	//MediaPlayer->OnMediaOpened.AddDynamic(this, &AOldManAnimationBall::Print);
	//设置循环
	MediaPlayer->SetLooping(Loop);
	//判断是否启用玩家输入
	if (PlayerInputCancel)
	{
		//Player->SetPlayerInput(false);
		UMyEventManager::GetEventManager()->TriggerCppEvent(UGlobalEventName::GetKey_Player_ChangeInputActive(),false);
	}
	//判断对话框是否自动播放
	if (myType == E_AniBallType::playAsText)
	{
		
	}
}

void AOldManAnimationBall::Print(FString text)
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AB_") + text);
}


void AOldManAnimationBall::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlayBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	if (OtherActor->Tags.Find(UGlobalTagName::Tag_Player) > -1)
	{
		if (Disposable)
		{
			//判断是否为一次性
			if (IsDisposable)
			{
				Disposable = false;
			}
			BeforePreparation();
			//Player = OtherActor->GetComponentByClass<AOldManCharacter>();
			switch (myType)
			{
			case E_AniBallType::playOnScene:
				PlayAniInScene();
				break;
			case E_AniBallType::playOnUI:
				PlayAniInUI();
				break;
			case E_AniBallType::playAsText:
				PlayText();
				break;
			default:
				UE_LOG(LogTemp, Warning, TEXT("AB_你不用，还不删，留着过年呢"));
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AB_你不用，还不删，留着过年呢"));
				break;
			}
		}
	}
}

