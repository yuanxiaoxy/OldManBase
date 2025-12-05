// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManAnimationBall.h"

//初始化
void AOldManAnimationBall::BeginPlay()
{
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
	//检测媒体声音组件是否存在
	if (MediaSound == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("AB_媒体声音组件不存在"));
	}
	//检测场景中播放的物体是否存在
	if (myType == E_AniBallType::playOnScene)
	{
		if (PlayWall == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("AB_场景中播放的物体不存在"));
		}
	}
}

//在场景中播放
void AOldManAnimationBall::PlayAniInScene()
{
	UE_LOG(LogTemp, Display, TEXT("AB_scene"));
}

//在UI界面上播放
void AOldManAnimationBall::PlayAniInUI()
{
	UE_LOG(LogTemp, Display, TEXT("AB_UI"));
}

//对话框
void AOldManAnimationBall::PlayText()
{
	UE_LOG(LogTemp, Display, TEXT("AB_text"));
}

//播放完毕
void AOldManAnimationBall::PlayOver()
{

}

//播放前准备
void AOldManAnimationBall::BeforePreparation()
{
	//启用计时器
	if (CountdownTime > 0)
	{
		UMonoManager::GetInstance()->SetTimeout(CountdownTime - BeginTime, this, &AOldManAnimationBall::PlayOver);
	}
	//设置循环
	MediaPlayer->SetLooping(Loop);
	//判断是否启用玩家输入
	
	//判断对话框是否自动播放

}



void AOldManAnimationBall::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlayBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	BeforePreparation();
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

