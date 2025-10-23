#include "OldManUI.h"

DEFINE_LOG_CATEGORY(OldManUI);

#define LOCTEXT_NAMESPACE "FOldManUI"

void FOldManUI::StartupModule()
{
	UE_LOG(OldManUI, Warning, TEXT("OldManUI module has started!"));
}

void FOldManUI::ShutdownModule()
{
	UE_LOG(OldManUI, Warning, TEXT("OldManUI module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOldManUI, OldManUI)