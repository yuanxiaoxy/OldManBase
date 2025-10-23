#include "OldManEnemy.h"

DEFINE_LOG_CATEGORY(OldManEnemy);

#define LOCTEXT_NAMESPACE "FOldManEnemy"

void FOldManEnemy::StartupModule()
{
	UE_LOG(OldManEnemy, Warning, TEXT("OldManEnemy module has started!"));
}

void FOldManEnemy::ShutdownModule()
{
	UE_LOG(OldManEnemy, Warning, TEXT("OldManEnemy module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FOldManEnemy, OldManEnemy)