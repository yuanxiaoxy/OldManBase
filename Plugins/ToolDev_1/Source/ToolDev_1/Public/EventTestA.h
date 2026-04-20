// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"


DECLARE_DELEGATE_TwoParams(FLoginDelegate, FString, FString);
class TOOLDEV_1_API SEventTestA : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEventTestA){}
		SLATE_EVENT(FLoginDelegate, OnStartLogin)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

private:
	FReply OnLoginButtonClicked();
	FLoginDelegate OnLogtnDelegate;
	TSharedPtr<SButton> LoginButtonPtr;
	TSharedPtr<SEditableTextBox> UserNamePtr;
	TSharedPtr<SEditableTextBox> PasswordPtr;
};
