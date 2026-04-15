// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/OldManTipLight.h"

// Sets default values
AOldManTipLight::AOldManTipLight()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;	//关闭Tick

	
}

// Called when the game starts or when spawned
void AOldManTipLight::BeginPlay()
{
	Super::BeginPlay();
	MyMeshComponent = FindComponentByClass<UStaticMeshComponent>();
	CanRunning = true;
	if (!OldManTips)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_可以移动老人头提示物体不存在"));
		CanRunning = false;
	}
	else
	{
		TipsMeshComponent = OldManTips->FindComponentByClass<UStaticMeshComponent>();
	}
	if (!OldManMove)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_可以移动老人头材质不存在"));
		CanRunning = false;
	}
	if (!OldManStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_不可移动老人头材质不存在"));
		CanRunning = false;
	}
	if (!LeftEyebrow)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_移动左眉毛材质不存在"));
		CanRunning = false;
	}
	if (!RightEyebrow)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_移动右眉毛材质不存在"));
		CanRunning = false;
	}
	if (!Chin)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_张嘴材质不存在"));
		CanRunning = false;
	}
	if (!TurnHeadLeft)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_左转头材质不存在"));
		CanRunning = false;
	}
	if (!TurnHeadRight)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_右转头材质不存在"));
		CanRunning = false;
	}
	if (!Wait)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossTP_待机材质不存在"));
		//找不到Wait材质时拿自身默认材质替代
		UMaterial* MaterialInterface = Cast<UMaterial>(MyMeshComponent->GetMaterial(0));
	}

}

void AOldManTipLight::SwitchCanMoveMat(bool CanMove)
{
	if (CanRunning)
	{
		if (CanMove)
		{
			TipsMeshComponent->SetMaterial(0, OldManMove);
		}
		else
		{
			TipsMeshComponent->SetMaterial(0, OldManStop);
		}
	}
}

void AOldManTipLight::SwitchOperationMat(ECurOperation TargetOperation)
{
	if (CanRunning)
	{
		switch (TargetOperation)
		{
			case ECurOperation::LeftEyebrow:
				MyMeshComponent->SetMaterial(0, LeftEyebrow);
				break;
			case ECurOperation::RightEyebrow:
				MyMeshComponent->SetMaterial(0, RightEyebrow);
				break;
			case ECurOperation::Chin:
				MyMeshComponent->SetMaterial(0, Chin);
				break;
			case ECurOperation::TurnHeadLeft:
				MyMeshComponent->SetMaterial(0, TurnHeadLeft);
				break;
			case ECurOperation::TurnHeadRight:
				MyMeshComponent->SetMaterial(0, TurnHeadRight);
				break;
			case ECurOperation::None:
			default:
				break;
		}
	}
}


