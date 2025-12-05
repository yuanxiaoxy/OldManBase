// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBase/OldManInterectItemBase.h"
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
	//动画球播放类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	E_AniBallType myType = E_AniBallType::playOnScene;
	//是否循环
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	bool Loop = false;
	//是否取消玩家输入
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType")
	bool PlayerInputCancel = true;
	//对话框是否自动播放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimationBallType",
		meta = (EditCondition = "myType == AniBallType::playAsText"))
	bool IsAutoText = true;



private:
	void PlayAniInScene();
	void PlayAniInUI();
	void PlayText();

protected:
	virtual void OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

};
