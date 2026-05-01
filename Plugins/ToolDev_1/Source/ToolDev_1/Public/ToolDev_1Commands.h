// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ToolDev_1Style.h"

class FToolDev_1Commands : public TCommands<FToolDev_1Commands>
{
public:

	FToolDev_1Commands()
		: TCommands<FToolDev_1Commands>(TEXT("ToolDev_1"), NSLOCTEXT("Contexts", "ToolDev_1", "ToolDev_1 Plugin"), NAME_None, FToolDev_1Style::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};