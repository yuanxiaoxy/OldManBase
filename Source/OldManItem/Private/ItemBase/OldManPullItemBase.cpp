// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/OldManPullItemBase.h"

void AOldManPullItemBase::HandleMouseData(const FVector& ViewDirection, float Intensity)
{
}

void AOldManPullItemBase::StartDragging()
{
	OnStartDragging();
}

void AOldManPullItemBase::StopDragging()
{
	OnStopDragging();
}
