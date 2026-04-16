// Fill out your copyright notice in the Description page of Project Settings.


//#include "TimerManager.h"
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
	if (!LeftEyebrowRun && !LeftEyebrowStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_移动左眉毛材质不存在"));
		CanRunning = false;
	}
	if (!RightEyebrowRun && !RightEyebrowStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_移动右眉毛材质不存在"));
		CanRunning = false;
	}
	if (!ChinRun && !ChinStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_张嘴材质不存在"));
		CanRunning = false;
	}
	if (!TurnHeadLeftRun && !TurnHeadLeftStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_左转头材质不存在"));
		CanRunning = false;
	}
	if (!TurnHeadRightRun && !TurnHeadRightStop)
	{
		UE_LOG(LogTemp, Error, TEXT("BossTP_右转头材质不存在"));
		CanRunning = false;
	}
	if (!Wait)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossTP_待机材质不存在"));
		//找不到Wait材质时拿自身默认材质替代
		UMaterial* MaterialInterface = Cast<UMaterial>(MyMeshComponent->GetMaterial(1));
	}
	if (!BiggerLightGreen && !BiggerLightRed && !SmallLightRed && !SmallLightGreen)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossTP_灯光不全"));
		CanRunning = false;
	}
	//初始化数值
	if (!CanRunning)return;
	CurType = ECurOperationType::LeftEyebrow;
	SmallLightGreenPointLight = SmallLightGreen->FindComponentByClass<UPointLightComponent>();
	SmallLightRedPointLight = SmallLightRed->FindComponentByClass<UPointLightComponent>();
	BiggerLightGreenPointLight = BiggerLightGreen->FindComponentByClass<UPointLightComponent>();
	BiggerLightRedPointLight = BiggerLightRed->FindComponentByClass<UPointLightComponent>();

	//SwitchCanMoveMat(OldManHeadCanMove);
}
 
void AOldManTipLight::SwitchCanMoveMat(bool CanMove)
{
	if (CanRunning)
	{
		OldManHeadCanMove = CanMove;
		if (CanMove)
		{
			MyMeshComponent->SetMaterial(CanMoveIndex, OldManMove);
			SmallLightGreenPointLight->SetVisibility(true);
			SmallLightRedPointLight->SetVisibility(false);
			BiggerLightGreenPointLight->SetVisibility(true);
			BiggerLightRedPointLight->SetVisibility(false);
			SwitchOperationMat(CurType);
		}
		else
		{
			MyMeshComponent->SetMaterial(CanMoveIndex, OldManStop);
			SmallLightGreenPointLight->SetVisibility(false);
			SmallLightRedPointLight->SetVisibility(true);
			BiggerLightGreenPointLight->SetVisibility(false);
			BiggerLightRedPointLight->SetVisibility(true);
			SwitchOperationMat(CurType);
		}
	}
}

void AOldManTipLight::SwitchOperationMat(ECurOperationType TargetOperation)
{
	if (CanRunning)
	{
		CurType = TargetOperation;
		switch (CurType)
		{
			case ECurOperationType::LeftEyebrow:
				MyMeshComponent->SetMaterial(BiggerLishtIndex, OldManHeadCanMove ? LeftEyebrowRun : LeftEyebrowStop);
				break;
			case ECurOperationType::RightEyebrow:
				MyMeshComponent->SetMaterial(BiggerLishtIndex, OldManHeadCanMove ? RightEyebrowRun : RightEyebrowStop);
				break;
			case ECurOperationType::Chin:
				MyMeshComponent->SetMaterial(BiggerLishtIndex, OldManHeadCanMove ? ChinRun : ChinStop);
				break;
			case ECurOperationType::TurnHeadLeft:
				MyMeshComponent->SetMaterial(BiggerLishtIndex, OldManHeadCanMove ? TurnHeadLeftRun : TurnHeadLeftStop);
				break;
			case ECurOperationType::TurnHeadRight:
				MyMeshComponent->SetMaterial(BiggerLishtIndex, OldManHeadCanMove ? TurnHeadRightRun : TurnHeadRightStop);
				break;
			case ECurOperationType::None:
			default:
				break;
		}
	}
}


