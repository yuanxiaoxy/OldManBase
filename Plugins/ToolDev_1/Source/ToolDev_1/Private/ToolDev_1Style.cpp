// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToolDev_1Style.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FToolDev_1Style::StyleInstance = nullptr;

void FToolDev_1Style::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FToolDev_1Style::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FToolDev_1Style::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ToolDev_1Style"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FToolDev_1Style::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ToolDev_1Style"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ToolDev_1")->GetBaseDir() / TEXT("Resources"));

	Style->Set("ToolDev_1.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

void FToolDev_1Style::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FToolDev_1Style::Get()
{
	return *StyleInstance;
}
