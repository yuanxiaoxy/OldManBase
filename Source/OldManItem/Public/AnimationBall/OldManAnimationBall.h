// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Runtime/MediaAssets/Public/MediaPlayer.h"
#include "Runtime/MediaAssets/Public/MediaTexture.h"
#include "Runtime/MediaAssets/Public/MediaSoundComponent.h"
#include "Runtime/MediaAssets/Public/FileMediaSource.h"
#include "Runtime/Engine/Classes/Components/AudioComponent.h"
#include "XyFrame/Public/MonoManager/MonoManager.h"
#include "EventManager/MyEventManager.h"
#include "Engine/StaticMeshActor.h"
#include "CoreMinimal.h"
#include "ItemBase/OldManInterectItemBase.h"
//#include "OldMan/Public/Character/OldManCharacter.h"
#include "OldManAnimationBall.generated.h"


UENUM(BlueprintType)
enum class E_AniBallType : uint8
{
	None UMETA(DisplayName = "None"),
	playOnScene UMETA(DisplayName = "Play On Scene"),
	playOnUI UMETA(DisplayName = "Play On UI"),
	playAsText UMETA(DisplayName = "Play As Text")
};
/**
 * 
 */
UCLASS()
class OLDMANITEM_API AOldManAnimationBall : public AOldManInterectItemBase
{
	GENERATED_BODY()
public:
	// 文件媒体源，指定视频文件路径[citation:1]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UFileMediaSource* FileMediaSource;

	// 媒体播放器组件
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMediaPlayer* MediaPlayer;

	// 媒体纹理，用于在材质中显示视频
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMediaTexture* MediaTexture;

	//在场景中播放的物体上的材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
	UMaterial* PlayWallMaterial;

	//动画球播放类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	E_AniBallType myType = E_AniBallType::playOnScene;
	//视频时长
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (ClampMin = 0.0f))
	float CountdownTime= 0.0f;
	//是否只生成物体
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (EditCondition = "myType == E_AniBallType::playOnScene"))
	bool IsCreateOnly = false;
	//从几秒开始播放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType", meta = (ClampMin = 0.0f))
	float BeginTime= 0.0f;
	//是否循环
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	bool Loop = false;
	//是否是一次性的
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	bool IsDisposable = true;
	//是否取消玩家输入
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	bool PlayerInputCancel = true;
	//对话框是否自动播放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType",
		meta = (EditCondition = "myType == E_AniBallType::playAsText"))
	bool IsAutoText = true;
	//在场景中播放的物体
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType",
		meta = (EditCondition = "myType == E_AniBallType::playOnScene"))
	AStaticMeshActor* PlayWall;

	//在场景中播放的物体上的材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallFade", meta = (EditCondition = "ShouldFadeIn == true"))
	UMaterialInstance* FadeInMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallFade")
	bool ShouldFadeIn = true;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartFadeIn();
	UFUNCTION(BlueprintCallable)
	void PlayVideoInUI();

private:
	//进入触发框的玩家
	//AOldManCharacter* Player;
	//一次性
	bool Disposable = true;

	UFUNCTION()
	void PlayAniInScene();
	UFUNCTION()
	void PlayAniInUI();
	UFUNCTION()
	void ChooseType();
	void PlayText();
	UFUNCTION()
	void PlayOver();

	void BeforePreparation();
	void Print(FString text);




protected:

	virtual void BeginPlay();//BeginPlay
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
