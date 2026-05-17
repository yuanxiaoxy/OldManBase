// Fill out your copyright notice in the Description page of Project Settings.


#include "PausePanel/PasuePanelButtonBase.h"

void UPasuePanelButtonBase::OnLocalSelected()
{
	BP_OnLocalSelected();

    if (OnHover)
    {
        PlayAnimation(OnHover);
    }
}

void UPasuePanelButtonBase::OnLocalDisSelected()
{
	BP_OnLocalDisSelected();

    if (OnHover)
    {
        QueuePlayAnimationReverse(OnHover);
    }
}

void UPasuePanelButtonBase::OnLocalClick()
{
	BP_OnLocalClick();
}

void UPasuePanelButtonBase::OnLocalIn()
{
    BP_OnLocalIn();
}
