// DeathSafeAreaManagerComponent.cpp
#include "OctreeDeathArea/DeathSafeAreaManagerComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDeathSafeAreaManagerComponent::UDeathSafeAreaManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;

    bDrawDebug = true;   // 默认开启调试绘制
}

void UDeathSafeAreaManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    RebuildOctreeInternal();
}

void UDeathSafeAreaManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearAllAreas();
    Super::EndPlay(EndPlayReason);
}

void UDeathSafeAreaManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bDrawDebug && GetWorld() && Octree.IsValid())
    {
        FRWScopeLock ReadLock(OctreeLock, SLT_ReadOnly);
        // 绘制所有死亡区域（红色）
        for (const auto& Area : DeathAreas)
        {
            DrawDebugBox(GetWorld(), Area.Bounds.Origin, Area.Bounds.BoxExtent, Area.DebugColor, false, -1.0f, 0, 2.0f);
        }
        // 绘制所有安全区域（绿色）
        for (const auto& Area : SafeAreas)
        {
            DrawDebugBox(GetWorld(), Area.Bounds.Origin, Area.Bounds.BoxExtent, Area.DebugColor, false, -1.0f, 0, 2.0f);
        }
    }
}

void UDeathSafeAreaManagerComponent::AddDeathArea(const FBox& Bounds, const FColor& DebugColor, const FString& AreaName)
{
    AddAreaToList(Bounds, DebugColor, AreaName, DeathAreas);
}

void UDeathSafeAreaManagerComponent::AddSafeArea(const FBox& Bounds, const FColor& DebugColor, const FString& AreaName)
{
    AddAreaToList(Bounds, DebugColor, AreaName, SafeAreas);
}

void UDeathSafeAreaManagerComponent::AddAreaToList(const FBox& Bounds, const FColor& Color, const FString& Name, TArray<FAreaElement>& TargetList)
{
    if (!Bounds.IsValid) return;

    FAreaElement NewElement;
    NewElement.Bounds = FBoxSphereBounds(Bounds);
    NewElement.DebugColor = Color;
    NewElement.AreaName = Name;

    {
        FRWScopeLock WriteLock(OctreeLock, SLT_Write);
        TargetList.Add(NewElement);
    }

    if (Octree.IsValid())
    {
        FRWScopeLock WriteLock(OctreeLock, SLT_Write);
        Octree->AddElement(NewElement);
    }
}

void UDeathSafeAreaManagerComponent::RemoveArea(const FBox& Bounds)
{
    if (!Bounds.IsValid) return;

    FBoxSphereBounds TargetBounds(Bounds);
    bool bRemoved = false;

    {
        FRWScopeLock WriteLock(OctreeLock, SLT_Write);
        bRemoved = (DeathAreas.RemoveAll([&](const FAreaElement& E) { return E.Bounds.GetBox() == TargetBounds.GetBox(); }) > 0) ||
            (SafeAreas.RemoveAll([&](const FAreaElement& E) { return E.Bounds.GetBox() == TargetBounds.GetBox(); }) > 0);
    }

    if (bRemoved)
    {
        RebuildOctreeInternal();
    }
}

void UDeathSafeAreaManagerComponent::ClearAllAreas()
{
    FRWScopeLock WriteLock(OctreeLock, SLT_Write);
    DeathAreas.Empty();
    SafeAreas.Empty();
    Octree.Reset();
}

void UDeathSafeAreaManagerComponent::RebuildOctree()
{
    RebuildOctreeInternal();
}

void UDeathSafeAreaManagerComponent::RebuildOctreeInternal()
{
    FRWScopeLock WriteLock(OctreeLock, SLT_Write);
    Octree = MakeUnique<FAreaOctree>(WorldBounds.GetCenter(), WorldBounds.GetExtent().GetMax());
    for (const auto& Area : DeathAreas) Octree->AddElement(Area);
    for (const auto& Area : SafeAreas) Octree->AddElement(Area);
}

void UDeathSafeAreaManagerComponent::UpdateActorLocation(AActor* Actor, const FVector& NewLocation)
{
    if (!Actor || !Octree.IsValid()) return;

    FVector* LastLocation = LastKnownLocations.Find(Actor);
    if (LastLocation && !HasMovedBeyondThreshold(*LastLocation, NewLocation))
    {
        return;
    }

    bool bIsSafe = IsLocationSafe(NewLocation);
    LastKnownLocations.Add(Actor, NewLocation);

    bool* LastSafeState = LastKnownSafeState.Find(Actor);
    bool bLastSafe = LastSafeState ? *LastSafeState : !bIsSafe;

    if (bIsSafe != bLastSafe)
    {
        LastKnownSafeState.Add(Actor, bIsSafe);
        OnAreaStateChanged.Broadcast(Actor, bIsSafe);
    }
}

bool UDeathSafeAreaManagerComponent::IsLocationSafe(const FVector& Location) const
{
    if (Mode == EOctreeMode::DefaultSafe)
    {
        return !IsLocationInDeathArea(Location);
    }
    else // DefaultDeath
    {
        return IsLocationInSafeArea(Location);
    }
}

FAreaQueryResult UDeathSafeAreaManagerComponent::QueryLocation(const FVector& Location) const
{
    FAreaQueryResult Result;
    if (!Octree.IsValid()) return Result;

    TArray<FAreaElement> FoundElements;
    FBoxCenterAndExtent QueryBounds(Location, FVector::ZeroVector);

    {
        FRWScopeLock ReadLock(OctreeLock, SLT_ReadOnly);
        Octree->FindElementsWithBoundsTest(QueryBounds, [&FoundElements](const FAreaElement& Element)
            {
                FoundElements.Add(Element);
            });
    }

    for (const auto& Element : FoundElements)
    {
        if (Element.Bounds.GetBox().IsInside(Location))
        {
            Result.bIsInArea = true;
            Result.AreaName = Element.AreaName;
            Result.AffectedBounds = Element.Bounds.GetBox();
            break;
        }
    }
    return Result;
}

bool UDeathSafeAreaManagerComponent::IsLocationInDeathArea(const FVector& Location) const
{
    for (const auto& Death : DeathAreas)
    {
        if (Death.Bounds.GetBox().IsInside(Location))
            return true;
    }
    return false;
}

bool UDeathSafeAreaManagerComponent::IsLocationInSafeArea(const FVector& Location) const
{
    for (const auto& Safe : SafeAreas)
    {
        if (Safe.Bounds.GetBox().IsInside(Location))
            return true;
    }
    return false;
}

bool UDeathSafeAreaManagerComponent::HasMovedBeyondThreshold(const FVector& OldLocation, const FVector& NewLocation) const
{
    return FVector::DistSquared(OldLocation, NewLocation) > (MovementThreshold * MovementThreshold);
}