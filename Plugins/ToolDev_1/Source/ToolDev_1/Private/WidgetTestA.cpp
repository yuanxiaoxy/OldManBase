// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetTestA.h"
#include "SlateOptMacros.h"
#include "EventTestA.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SWidgetTestA::Construct(const FArguments& InArgs)
{
	FString text = InArgs._InText.Get();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("Attribute is %s"), *text));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Attribute is %s (GEngine not available)"), *text);
	}


	ChildSlot
	[
		SNew(SEventTestA).OnStartLogin(this, &SWidgetTestA::OnLogin)
		/*SNew(SVerticalBox)
		+SVerticalBox::Slot()
		[
			SNew(SButton).OnClicked(this, &SWidgetTestA::OnLoginButtonClicked)
		]
		+SVerticalBox::Slot()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
			]
			+SHorizontalBox::Slot()
			[
				SNew(SButton)
			]
		]*/
	];
}
FReply SWidgetTestA::OnLoginButtonClicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString::Printf(TEXT("OnBUttonClicked")));
	UE_LOG(LogTemp, Warning, TEXT("OnBUttonClicked"));
	return FReply::Handled();
}

void SWidgetTestA::OnLogin(FString usn, FString pwd)
{
	UE_LOG(LogTemp, Warning, TEXT("OnLoginClicked"));
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
