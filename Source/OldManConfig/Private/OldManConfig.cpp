#include "OldManConfig.h"

DEFINE_LOG_CATEGORY(OldManConfig);

#define LOCTEXT_NAMESPACE "FOldManConfig"

void FOldManConfig::StartupModule()
{
	UE_LOG(OldManConfig, Warning, TEXT("OldManConfig module has started!"));
}

void FOldManConfig::ShutdownModule()
{
	UE_LOG(OldManConfig, Warning, TEXT("OldManConfig module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOldManConfig, OldManConfig)