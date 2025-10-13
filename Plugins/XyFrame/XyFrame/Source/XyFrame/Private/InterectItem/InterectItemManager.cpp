// Fill out your copyright notice in the Description page of Project Settings.

#include "InterectItem/InterectItemManager.h"
#include "InterectItem/InterectItemBase.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UInterectItemManager::UInterectItemManager()
{
    // Constructor
}

UInterectItemManager::~UInterectItemManager()
{
    // Clean up resources
    RegisteredInteractItems.Empty();
    InteractItemsById.Empty();
    InteractItemsByClass.Empty();
    ActiveInteractions.Empty();
}

void UInterectItemManager::InitializeInteractManager()
{
    UE_LOG(LogTemp, Log, TEXT("Interact Item Manager Initialized"));
}

void UInterectItemManager::InitializeSingleton()
{
    InitializeInteractManager();
}

void UInterectItemManager::RegisterInteractItem(AInterectItemBase* InteractItem)
{
    if (InteractItem && IsValid(InteractItem))
    {
        if (!RegisteredInteractItems.Contains(InteractItem))
        {
            RegisteredInteractItems.Add(InteractItem);
            UpdateItemMappings(InteractItem, true);

            // Bind events
            InteractItem->OnInteractStateChanged.AddDynamic(this, &UInterectItemManager::OnInteractItemStateChanged);
            InteractItem->OnInteractTriggered.AddDynamic(this, &UInterectItemManager::OnInteractItemTriggered);

            UE_LOG(LogTemp, Log, TEXT("Registered interact item: %s (ID: %s)"),
                *InteractItem->GetName(), *InteractItem->InteractItemId);
        }
    }
}

void UInterectItemManager::UnregisterInteractItem(AInterectItemBase* InteractItem)
{
    if (InteractItem)
    {
        if (RegisteredInteractItems.Contains(InteractItem))
        {
            RegisteredInteractItems.Remove(InteractItem);
            UpdateItemMappings(InteractItem, false);

            // Remove from active interactions
            for (auto& Pair : ActiveInteractions)
            {
                Pair.Value.Remove(InteractItem);
            }

            UE_LOG(LogTemp, Log, TEXT("Unregistered interact item: %s"), *InteractItem->GetName());
        }
    }
}

AInterectItemBase* UInterectItemManager::FindInteractItemById(const FString& ItemId) const
{
    if (AInterectItemBase* const* FoundItem = InteractItemsById.Find(ItemId))
    {
        return *FoundItem;
    }
    return nullptr;
}

AInterectItemBase* UInterectItemManager::FindInteractItemByTag(FName Tag) const
{
    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item && Item->ActorHasTag(Tag))
        {
            return Item;
        }
    }
    return nullptr;
}

TArray<AInterectItemBase*> UInterectItemManager::GetAllInteractItems() const
{
    return RegisteredInteractItems;
}

TArray<AInterectItemBase*> UInterectItemManager::GetInteractItemsByClass(TSubclassOf<AInterectItemBase> ItemClass) const
{
    if (const TArray<AInterectItemBase*>* FoundItems = InteractItemsByClass.Find(ItemClass))
    {
        return *FoundItems;
    }
    return TArray<AInterectItemBase*>();
}

TArray<AInterectItemBase*> UInterectItemManager::GetInteractItemsByState(EInteractState State) const
{
    TArray<AInterectItemBase*> Result;
    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item && Item->CurrentState == State)
        {
            Result.Add(Item);
        }
    }
    return Result;
}

TArray<AInterectItemBase*> UInterectItemManager::GetInteractItemsByMode(EInteractMode Mode) const
{
    TArray<AInterectItemBase*> Result;
    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item && Item->InteractMode == Mode)
        {
            Result.Add(Item);
        }
    }
    return Result;
}

AInterectItemBase* UInterectItemManager::GetNearestInteractItem(const FVector& Location, const FInteractFilter& Filter) const
{
    AInterectItemBase* NearestItem = nullptr;
    float NearestDistance = MAX_FLT;

    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (IsValidForFilter(Item, Filter, &Location))
        {
            float Distance = FVector::Distance(Item->GetActorLocation(), Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestItem = Item;
            }
        }
    }

    return NearestItem;
}

TArray<AInterectItemBase*> UInterectItemManager::GetInteractItemsNearActor(AActor* Actor, float Radius, const FInteractFilter& Filter) const
{
    TArray<AInterectItemBase*> Result;

    if (!Actor)
        return Result;

    FVector ActorLocation = Actor->GetActorLocation();

    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (IsValidForFilter(Item, Filter, &ActorLocation))
        {
            float Distance = FVector::Distance(Item->GetActorLocation(), ActorLocation);
            if (Distance <= Radius)
            {
                Result.Add(Item);
            }
        }
    }

    return Result;
}

TArray<AInterectItemBase*> UInterectItemManager::GetAvailableInteractItemsForActor(AActor* Actor) const
{
    TArray<AInterectItemBase*> Result;

    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item && Item->CanInteract(Actor))
        {
            Result.Add(Item);
        }
    }

    return Result;
}

bool UInterectItemManager::StartInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData)
{
    if (!InteractingActor || !InteractItem)
        return false;

    if (InteractItem->CanInteract(InteractingActor))
    {
        InteractItem->StartInteract(InteractingActor, InteractData);

        // Record active interaction
        TArray<AInterectItemBase*>& ActorInteractions = ActiveInteractions.FindOrAdd(InteractingActor);
        if (!ActorInteractions.Contains(InteractItem))
        {
            ActorInteractions.Add(InteractItem);
        }

        return true;
    }

    return false;
}

bool UInterectItemManager::EndInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->EndInteract(InteractingActor);

    // Remove from active interactions
    TArray<AInterectItemBase*>* ActorInteractions = ActiveInteractions.Find(InteractingActor);
    if (ActorInteractions)
    {
        ActorInteractions->Remove(InteractItem);
        if (ActorInteractions->Num() == 0)
        {
            ActiveInteractions.Remove(InteractingActor);
        }
    }

    return true;
}

bool UInterectItemManager::UpdateInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->UpdateInteract(InteractData);
    return true;
}

bool UInterectItemManager::CancelInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->CancelInteract(InteractingActor);

    // Remove from active interactions
    TArray<AInterectItemBase*>* ActorInteractions = ActiveInteractions.Find(InteractingActor);
    if (ActorInteractions)
    {
        ActorInteractions->Remove(InteractItem);
        if (ActorInteractions->Num() == 0)
        {
            ActiveInteractions.Remove(InteractingActor);
        }
    }

    return true;
}

bool UInterectItemManager::StartMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->StartMouseControl(InteractData);
    return true;
}

bool UInterectItemManager::UpdateMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->UpdateMouseControl(InteractData);
    return true;
}

bool UInterectItemManager::EndMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem)
{
    if (!InteractingActor || !InteractItem)
        return false;

    InteractItem->EndMouseControl();
    return true;
}

void UInterectItemManager::SetAllInteractItemsEnabled(bool bEnabled)
{
    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item)
        {
            Item->SetEnabled(bEnabled);
        }
    }
}

void UInterectItemManager::SetInteractItemsEnabledByClass(TSubclassOf<AInterectItemBase> ItemClass, bool bEnabled)
{
    TArray<AInterectItemBase*> Items = GetInteractItemsByClass(ItemClass);
    for (AInterectItemBase* Item : Items)
    {
        if (Item)
        {
            Item->SetEnabled(bEnabled);
        }
    }
}

void UInterectItemManager::ResetAllInteractItems()
{
    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item)
        {
            Item->SetInteractState(EInteractState::Idle);
            Item->SetEnabled(true);
        }
    }
}

FInteractManagerStats UInterectItemManager::GetStats() const
{
    FInteractManagerStats Stats;

    Stats.TotalItems = RegisteredInteractItems.Num();

    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item && Item->bIsEnabled)
        {
            Stats.EnabledItems++;
        }

        if (Item)
        {
            Stats.ItemsByState[Item->CurrentState]++;
            Stats.ItemsByMode[Item->InteractMode]++;
        }
    }

    // Calculate active interactions count
    Stats.ActiveInteractions = 0;
    for (const auto& Pair : ActiveInteractions)
    {
        Stats.ActiveInteractions += Pair.Value.Num();
    }

    return Stats;
}

void UInterectItemManager::PrintAllInteractItems() const
{
    UE_LOG(LogTemp, Log, TEXT("=== All Interact Items (%d) ==="), RegisteredInteractItems.Num());

    for (AInterectItemBase* Item : RegisteredInteractItems)
    {
        if (Item)
        {
            FString Status = Item->bIsEnabled ? "Enabled" : "Disabled";
            UE_LOG(LogTemp, Log, TEXT("  %s (ID: %s) - State: %s, Mode: %s, %s"),
                *Item->GetName(),
                *Item->InteractItemId,
                *UEnum::GetValueAsString(Item->CurrentState),
                *UEnum::GetValueAsString(Item->InteractMode),
                *Status);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Interact Items ==="));
}

void UInterectItemManager::PrintStats() const
{
    FInteractManagerStats Stats = GetStats();

    UE_LOG(LogTemp, Log, TEXT("=== Interact Manager Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Items: %d"), Stats.TotalItems);
    UE_LOG(LogTemp, Log, TEXT("Enabled Items: %d"), Stats.EnabledItems);
    UE_LOG(LogTemp, Log, TEXT("Active Interactions: %d"), Stats.ActiveInteractions);

    UE_LOG(LogTemp, Log, TEXT("Items by State:"));
    for (const auto& Pair : Stats.ItemsByState)
    {
        if (Pair.Value > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *UEnum::GetValueAsString(Pair.Key), Pair.Value);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Items by Mode:"));
    for (const auto& Pair : Stats.ItemsByMode)
    {
        if (Pair.Value > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *UEnum::GetValueAsString(Pair.Key), Pair.Value);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("=== End Stats ==="));
}

void UInterectItemManager::OnInteractItemStateChanged(AActor* InteractItem, EInteractState NewState)
{
    // Can add global state change logic here
    UE_LOG(LogTemp, Verbose, TEXT("Interact item state changed: %s -> %s"),
        *InteractItem->GetName(), *UEnum::GetValueAsString(NewState));
}

void UInterectItemManager::OnInteractItemTriggered(AActor* InteractItem, AActor* InteractingActor, const FInteractData& InteractData)
{
    // Can add global interaction trigger logic here
    UE_LOG(LogTemp, Log, TEXT("Interact item triggered: %s by %s"),
        *InteractItem->GetName(), *InteractingActor->GetName());
}

void UInterectItemManager::UpdateItemMappings(AInterectItemBase* InteractItem, bool bAdd)
{
    if (!InteractItem)
        return;

    if (bAdd)
    {
        // Add to ID mapping
        if (!InteractItem->InteractItemId.IsEmpty())
        {
            InteractItemsById.Add(InteractItem->InteractItemId, InteractItem);
        }

        // Add to class mapping
        TSubclassOf<AInterectItemBase> ItemClass = InteractItem->GetClass();
        TArray<AInterectItemBase*>& ClassArray = InteractItemsByClass.FindOrAdd(ItemClass);
        ClassArray.Add(InteractItem);
    }
    else
    {
        // Remove from ID mapping
        if (!InteractItem->InteractItemId.IsEmpty())
        {
            InteractItemsById.Remove(InteractItem->InteractItemId);
        }

        // Remove from class mapping
        TSubclassOf<AInterectItemBase> ItemClass = InteractItem->GetClass();
        TArray<AInterectItemBase*>* ClassArray = InteractItemsByClass.Find(ItemClass);
        if (ClassArray)
        {
            ClassArray->Remove(InteractItem);
            if (ClassArray->Num() == 0)
            {
                InteractItemsByClass.Remove(ItemClass);
            }
        }
    }
}

bool UInterectItemManager::IsValidForFilter(AInterectItemBase* Item, const FInteractFilter& Filter, const FVector* QueryLocation) const
{
    if (!Item || !Item->bIsEnabled)
        return false;

    // Class filter
    if (Filter.ItemClass && !Item->IsA(Filter.ItemClass))
        return false;

    // State filter
    if (Item->CurrentState != Filter.RequiredState)
        return false;

    // Enabled state filter
    if (Filter.bEnabledOnly && !Item->bIsEnabled)
        return false;

    // Distance filter
    if (QueryLocation && Filter.MaxDistance > 0)
    {
        float Distance = FVector::Distance(Item->GetActorLocation(), *QueryLocation);
        if (Distance > Filter.MaxDistance)
            return false;
    }

    return true;
}

UWorld* UInterectItemManager::GetWorld() const
{
    // Get World context from external
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                return Context.World();
            }
        }
    }
    return nullptr;
}