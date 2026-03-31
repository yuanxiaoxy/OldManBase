// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingPanel/SettingPanel.h"


void USettingPanel::UpdateBGMToUI(float BGMVol)
{
	BGMVolume = BGMVol;
}

void USettingPanel::UpdateEffectToUI(float EffectVol)
{
	EffectVolume = EffectVol;
}

void USettingPanel::UpdateBGMToGame()
{

}

void USettingPanel::UpdateEffectToGame()
{

}

void USettingPanel::OnChangeTabSwitch(int index)
{
	UI_OnChangeTabSwitch(index);
}

