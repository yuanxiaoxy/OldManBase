// ADeathAreaVolume.cpp
#include "OctreeDeathArea/DeathAreaVolume.h"
#include "OctreeDeathArea/DeathSafeAreaManagerComponent.h"
#include "Engine/World.h"
#include "Components/BrushComponent.h"

ADeathAreaVolume::ADeathAreaVolume()
{
    GetBrushComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetBrushComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

#if WITH_EDITOR
void ADeathAreaVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UpdateDeathAreaInManager();
}
#endif

FBox ADeathAreaVolume::GetDeathBounds() const
{
    FBox Bounds(EForceInit::ForceInitToZero);
    if (GetBrushComponent())
    {
        Bounds = GetBrushComponent()->Bounds.GetBox();
    }
    return Bounds;
}

void ADeathAreaVolume::UpdateDeathAreaInManager()
{
    if (!bIsActive)
    {
        UnregisterFromManager();
        return;
    }
    RegisterToManager();
}

void ADeathAreaVolume::BeginPlay()
{
    Super::BeginPlay();
    RegisterToManager();
}

void ADeathAreaVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromManager();
    Super::EndPlay(EndPlayReason);
}

UDeathSafeAreaManagerComponent* ADeathAreaVolume::GetManager()
{
    if (CachedManager.IsValid()) return CachedManager.Get();

    UWorld* World = GetWorld();
    if (!World) return nullptr;

    for (TObjectIterator<UDeathSafeAreaManagerComponent> It; It; ++It)
    {
        if (It->GetWorld() == World)
        {
            CachedManager = *It;
            return CachedManager.Get();
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* ManagerActor = World->SpawnActor<AActor>(SpawnParams);
    if (ManagerActor)
    {
        UDeathSafeAreaManagerComponent* NewManager = NewObject<UDeathSafeAreaManagerComponent>(ManagerActor);
        NewManager->RegisterComponent();
        CachedManager = NewManager;
        return NewManager;
    }
    return nullptr;
}

void ADeathAreaVolume::RegisterToManager()
{
    if (UDeathSafeAreaManagerComponent* Manager = GetManager())
    {
        Manager->AddDeathArea(GetDeathBounds(), DebugColor, AreaName);
    }
}

void ADeathAreaVolume::UnregisterFromManager()
{
    if (CachedManager.IsValid())
    {
        CachedManager->RemoveArea(GetDeathBounds());
    }
}