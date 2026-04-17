// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss/OldManMobileCamera.h"
#include "MonoManager/MonoManager.h"

AOldManMobileCamera::AOldManMobileCamera()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AOldManMobileCamera::BeginPlay()
{
	Super::BeginPlay();
	if (!SpotLight)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossTP_聚光灯不存在"));

	}
	mySpotLight = SpotLight->FindComponentByClass<USpotLightComponent>();
}

void AOldManMobileCamera::Tick(float DeltaTime)
{
}

void AOldManMobileCamera::BeginScan(float time)
{
	//打开聚光灯
	mySpotLight->SetVisibility(true);
	UMonoManager* monoManager = UMonoManager::GetMonoManager();
	monoManager->SetInterval(time, "ScanTimer", this, &AOldManMobileCamera::CloseLight);
}

void AOldManMobileCamera::BeginScanWithoutParm()
{
	//打开聚光灯
	mySpotLight->SetVisibility(true);

}

void AOldManMobileCamera::CloseLight()
{
	mySpotLight->SetVisibility(false);
}
