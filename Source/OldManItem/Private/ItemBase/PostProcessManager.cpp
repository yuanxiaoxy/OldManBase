// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBase/PostProcessManager.h"

void APostProcessManager::StartPostProcessAnim()
{
    BP_StartPostProcessAnim();
}

void APostProcessManager::StopPostProcessAnim()
{
    BP_StopPostProcessAnim();
}

void APostProcessManager::SetLightStrength(float Value)
{
    BP_SetLightStrength(Value);
}

void APostProcessManager::Fade2None()
{
    BP_Fade2None();
}

void APostProcessManager::FlashRed(float Time)
{
    BP_FlashRed(Time);
}

void APostProcessManager::FlashGreen(float Time)
{
    BP_FlashGreen(Time);
}

