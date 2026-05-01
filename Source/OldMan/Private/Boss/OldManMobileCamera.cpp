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
}

void AOldManMobileCamera::Tick(float DeltaTime)
{
}

void AOldManMobileCamera::BeginScan(float time)
{
	UMonoManager* monoManager = UMonoManager::GetMonoManager();
	Open();
	monoManager->SetInterval(time, "ScanTimer", this, &AOldManMobileCamera::CloseLight);
}

void AOldManMobileCamera::BeginScanWithoutParm()
{
	Open();
}

void AOldManMobileCamera::CloseLight()
{
	Close();
}
