// Fill out your copyright notice in the Description page of Project Settings.


#include "Area/GravityChangeArea.h"
//#include "Character/OldManPersonPlayerController.h"


void AGravityChangeArea::OnOverlayBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Tags.Find(UGlobalTagName::Tag_Player) > -1)
	{
		OnEnterTrigger(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

		UMyEventManager::GetInstance()->TriggerCppEvent(UGlobalEventName::Key_Player_OnChangeGrivity, GetInOrOutCustomGravityArea);
		//A* PlayerController = GetWorld()->GetFirstPlayerController();
	}
}

void AGravityChangeArea::OnOverlayEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}
