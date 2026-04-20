// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToolDev_1.h"
#include "ToolDev_1Style.h"
#include "ToolDev_1Commands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "WidgetTestA.h"
#include "EventTestA.h"

static const FName ToolDev_1TabName("ToolDev_1");

#define LOCTEXT_NAMESPACE "FToolDev_1Module"

void FToolDev_1Module::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FToolDev_1Style::Initialize();
	FToolDev_1Style::ReloadTextures();

	FToolDev_1Commands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FToolDev_1Commands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FToolDev_1Module::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FToolDev_1Module::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ToolDev_1TabName, FOnSpawnTab::CreateRaw(this, &FToolDev_1Module::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FToolDev_1TabTitle", "ToolDev_1"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FToolDev_1Module::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FToolDev_1Style::Shutdown();

	FToolDev_1Commands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ToolDev_1TabName);
}

TSharedRef<SDockTab> FToolDev_1Module::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FToolDev_1Module::OnSpawnPluginTab")),
		FText::FromString(TEXT("ToolDev_1.cpp"))
		);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SWidgetTestA).InText(FString("Hello Slate"))
			// Put your tab content here!
			/*SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]*/
		];
}

void FToolDev_1Module::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ToolDev_1TabName);
}

void FToolDev_1Module::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FToolDev_1Commands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FToolDev_1Commands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FToolDev_1Module, ToolDev_1)