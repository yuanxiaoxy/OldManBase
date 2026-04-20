// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class TOOLDEV_1_API SWidgetTestA : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWidgetTestA) {}
	SLATE_ATTRIBUTE(FString, InText);
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnLoginButtonClicked();
	void OnLogin(FString usn, FString pwd);
};
