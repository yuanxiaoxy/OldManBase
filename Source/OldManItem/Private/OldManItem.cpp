#include "OldManItem.h"

DEFINE_LOG_CATEGORY(OldManItem);

#define LOCTEXT_NAMESPACE "FOldManItem"

void FOldManItem::StartupModule()
{
	UE_LOG(OldManItem, Warning, TEXT("OldManItem module has started!"));
}

void FOldManItem::ShutdownModule()
{
	UE_LOG(OldManItem, Warning, TEXT("OldManItem module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOldManItem, OldManItem)