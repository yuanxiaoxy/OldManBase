// ASafeAreaVolume.cpp
#include "OctreeDeathArea/SafeAreaVolume.h"
#include "OctreeDeathArea/DeathSafeAreaManagerComponent.h"
#include "Engine/World.h"
#include "Components/BrushComponent.h"

ASafeAreaVolume::ASafeAreaVolume()
{
    GetBrushComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    GetBrushComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
}

#if WITH_EDITOR
void ASafeAreaVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UpdateSafeAreaInManager();
}
#endif

FBox ASafeAreaVolume::GetSafeBounds() const
{
    FBox Bounds(EForceInit::ForceInitToZero);
    if (GetBrushComponent())
    {
        Bounds = GetBrushComponent()->Bounds.GetBox();
    }
    return Bounds;
}

void ASafeAreaVolume::UpdateSafeAreaInManager()
{
    if (!bIsActive)
    {
        UnregisterFromManager();
        return;
    }
    RegisterToManager();
}

void ASafeAreaVolume::BeginPlay()
{
    Super::BeginPlay();
    RegisterToManager();
}

void ASafeAreaVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromManager();
    Super::EndPlay(EndPlayReason);
}

UDeathSafeAreaManagerComponent* ASafeAreaVolume::GetManager()
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

void ASafeAreaVolume::RegisterToManager()
{
    if (UDeathSafeAreaManagerComponent* Manager = GetManager())
    {
        Manager->AddSafeArea(GetSafeBounds(), DebugColor, AreaName);
    }
}

void ASafeAreaVolume::UnregisterFromManager()
{
    if (CachedManager.IsValid())
    {
        CachedManager->RemoveArea(GetSafeBounds());
    }
}