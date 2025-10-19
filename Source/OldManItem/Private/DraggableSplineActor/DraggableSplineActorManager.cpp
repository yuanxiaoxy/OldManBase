// Fill out your copyright notice in the Description page of Project Settings.


#include "DraggableSplineActor/DraggableSplineActorManager.h"

ADraggableSplineActorManager::ADraggableSplineActorManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADraggableSplineActorManager::BeginPlay()
{
	Super::BeginPlay();
}

void ADraggableSplineActorManager::ResetDraggableSplineActorPos(FString GroupName)
{
	if (DraggableActorMap.Contains(GroupName))
	{
		for (ADraggableSplineActor* draggableSplineActor : DraggableActorMap[GroupName].DraggableSplineActors)
		{
			draggableSplineActor->SetStartPosition();
		}
	}
}
