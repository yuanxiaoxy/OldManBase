// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManAnimationBall.h"

void AOldManAnimationBall::PlayAniInScene()
{
	UE_LOG(LogTemp, Display, TEXT("scene"));
}

void AOldManAnimationBall::PlayAniInUI()
{
	UE_LOG(LogTemp, Display, TEXT("UI"));
}

void AOldManAnimationBall::PlayText()
{
	UE_LOG(LogTemp, Display, TEXT("text"));
}

void AOldManAnimationBall::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnOverlayBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
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
			UE_LOG(LogTemp, Warning, TEXT("你不用，还不删，留着过年呢"));
			break;
	}
}
