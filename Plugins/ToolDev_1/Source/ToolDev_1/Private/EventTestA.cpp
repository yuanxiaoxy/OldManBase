// Fill out your copyright notice in the Description page of Project Settings.


#include "EventTestA.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
#define LOCTEXT_NAMESPACE "SEventTest"
void SEventTestA::Construct(const FArguments& InArgs)
{
	OnLogtnDelegate = InArgs._OnStartLogin;

	ChildSlot.Padding(50,50,50,50)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(UserNamePtr, SEditableTextBox)
				.HintText(LOCTEXT("Username_Hint", "请输入账号"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(PasswordPtr, SEditableTextBox)
			.IsPassword(true)
				.HintText(LOCTEXT("Password_Hint", "请输入密码"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(LoginButtonPtr, SButton)
			.OnClicked(this, &SEventTestA::OnLoginButtonClicked)
			.Text(LOCTEXT("Login", "登录"))
		]
	];
	
}

FReply SEventTestA::OnLoginButtonClicked()
{
	FString usn = UserNamePtr->GetText().ToString();
	FString pwd = PasswordPtr->GetText().ToString();

	OnLogtnDelegate.ExecuteIfBound(usn, pwd);
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
