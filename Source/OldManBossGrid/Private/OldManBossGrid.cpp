#include "OldManBossGrid.h"

DEFINE_LOG_CATEGORY(OldManBossGrid);

#define LOCTEXT_NAMESPACE "FOldManBossGrid"

void FOldManBossGrid::StartupModule()
{
	UE_LOG(OldManBossGrid, Warning, TEXT("OldManBossGrid module has started!"));
}

void FOldManBossGrid::ShutdownModule()
{
	UE_LOG(OldManBossGrid, Warning, TEXT("OldManBossGrid module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOldManBossGrid, OldManBossGrid)