// Copyright Epic Games, Inc. All Rights Reserved.

#include "ToolDev_1Commands.h"

#define LOCTEXT_NAMESPACE "FToolDev_1Module"

void FToolDev_1Commands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ToolDev_1", "Bring up ToolDev_1 window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
