// Fill out your copyright notice in the Description page of Project Settings.

#include "InterectItem/InterectItemBase.h"
#include "InterectItem/InterectItemManager.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"

AInterectItemBase::AInterectItemBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // Create trigger sphere component
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    TriggerSphere->SetupAttachment(RootComponent);
    TriggerSphere->SetCollisionProfileName(TEXT("Trigger"));
    TriggerSphere->SetGenerateOverlapEvents(true);
    TriggerSphere->SetSphereRadius(InteractRange);

    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AInterectItemBase::OnTriggerBeginOverlap);
    TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AInterectItemBase::OnTriggerEndOverlap);
}

void AInterectItemBase::BeginPlay()
{
    Super::BeginPlay();

    // Auto-register to manager
    if (bAutoRegisterToManager)
    {
        if (UInterectItemManager* Manager = UInterectItemManager::GetInteractManager())
        {
            Manager->RegisterInteractItem(this);
        }
    }

    // Set initial state
    SetInteractState(bIsEnabled ? EInteractState::Idle : EInteractState::Disabled);
}

void AInterectItemBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Unregister from manager
    if (UInterectItemManager* Manager = UInterectItemManager::GetInteractManager())
    {
        Manager->UnregisterInteractItem(this);
    }

    Super::EndPlay(EndPlayReason);
}

void AInterectItemBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto trigger mode state update
    if (bIsEnabled && InteractMode == EInteractMode::AutoTrigger)
    {
        // Check if there are valid interactors in range
        bool bHasValidInteractor = false;
        for (AActor* Actor : CurrentInteractingActors)
        {
            if (IsValidInteractor(Actor) && IsInInteractRange(Actor))
            {
                bHasValidInteractor = true;
                break;
            }
        }

        // Update state
        if (bHasValidInteractor && CurrentState == EInteractState::Idle)
        {
            SetInteractState(EInteractState::Ready);
        }
        else if (!bHasValidInteractor && CurrentState == EInteractState::Ready)
        {
            SetInteractState(EInteractState::Idle);
        }
    }
}

void AInterectItemBase::SetInteractState(EInteractState NewState)
{
    if (CurrentState != NewState)
    {
        EInteractState OldState = CurrentState;
        CurrentState = NewState;

        // Trigger state change event
        OnInteractStateChanged.Broadcast(this, NewState);

        // State-specific logic
        switch (NewState)
        {
        case EInteractState::Disabled:
            // Clear all interactions when disabled
            for (AActor* Actor : CurrentInteractingActors)
            {
                CancelInteract(Actor);
            }
            CurrentInteractingActors.Empty();
            break;
        case EInteractState::Ready:
            // Ready state logic
            break;
        case EInteractState::Active:
            // Active state logic
            break;
        case EInteractState::Completed:
            // Completed state logic
            break;
        }
    }
}

void AInterectItemBase::SetEnabled(bool bEnabled)
{
    if (bIsEnabled != bEnabled)
    {
        bIsEnabled = bEnabled;
        SetInteractState(bEnabled ? EInteractState::Idle : EInteractState::Disabled);
    }
}

bool AInterectItemBase::CanInteract(AActor* InteractingActor) const
{
    if (!bIsEnabled || CurrentState == EInteractState::Disabled)
        return false;

    if (!IsValidInteractor(InteractingActor))
        return false;

    if (!IsInInteractRange(InteractingActor))
        return false;

    return true;
}

FText AInterectItemBase::GetInteractPrompt() const
{
    if (!bIsEnabled)
        return FText::FromString("Disabled");

    switch (CurrentState)
    {
    case EInteractState::Ready:
        return FText::FromString("Press E to interact");
    case EInteractState::Active:
        return FText::FromString("Interacting...");
    case EInteractState::Completed:
        return FText::FromString("Completed");
    default:
        return FText::FromString("");
    }
}

void AInterectItemBase::StartInteract_Implementation(AActor* InteractingActor, const FInteractData& InteractData)
{
    if (CanInteract(InteractingActor))
    {
        CurrentInteractingActors.Add(InteractingActor);
        SetInteractState(EInteractState::Active);

        // Trigger interaction event
        OnInteractTriggered.Broadcast(this, InteractingActor, InteractData);

        UE_LOG(LogTemp, Log, TEXT("Started interaction: %s with %s"),
            *GetName(), *InteractingActor->GetName());
    }
}

void AInterectItemBase::EndInteract_Implementation(AActor* InteractingActor)
{
    CurrentInteractingActors.Remove(InteractingActor);

    if (CurrentInteractingActors.Num() == 0)
    {
        SetInteractState(EInteractState::Completed);
    }

    UE_LOG(LogTemp, Log, TEXT("Ended interaction: %s with %s"),
        *GetName(), *InteractingActor->GetName());
}

void AInterectItemBase::UpdateInteract_Implementation(const FInteractData& InteractData)
{
    // Base implementation empty, can be overridden in subclasses
}

void AInterectItemBase::CancelInteract_Implementation(AActor* InteractingActor)
{
    CurrentInteractingActors.Remove(InteractingActor);

    if (CurrentInteractingActors.Num() == 0)
    {
        SetInteractState(EInteractState::Idle);
    }

    UE_LOG(LogTemp, Log, TEXT("Cancelled interaction: %s with %s"),
        *GetName(), *InteractingActor->GetName());
}

void AInterectItemBase::StartMouseControl_Implementation(const FInteractData& InteractData)
{
    // Base implementation empty, can be overridden in subclasses
    UE_LOG(LogTemp, Log, TEXT("Started mouse control: %s"), *GetName());
}

void AInterectItemBase::UpdateMouseControl_Implementation(const FInteractData& InteractData)
{
    // Base implementation empty, can be overridden in subclasses
}

void AInterectItemBase::EndMouseControl_Implementation()
{
    // Base implementation empty, can be overridden in subclasses
    UE_LOG(LogTemp, Log, TEXT("Ended mouse control: %s"), *GetName());
}

float AInterectItemBase::GetDistanceToActor(AActor* OtherActor) const
{
    if (!OtherActor)
        return MAX_FLT;

    return FVector::Distance(GetActorLocation(), OtherActor->GetActorLocation());
}

bool AInterectItemBase::IsInInteractRange(AActor* OtherActor) const
{
    return GetDistanceToActor(OtherActor) <= InteractRange;
}

bool AInterectItemBase::IsInteractingWithActor(AActor* Actor) const
{
    return CurrentInteractingActors.Contains(Actor);
}

void AInterectItemBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValidInteractor(OtherActor))
    {
        // Auto trigger mode starts interaction immediately
        if (InteractMode == EInteractMode::AutoTrigger && CanInteract(OtherActor))
        {
            FInteractData InteractData(OtherActor, SweepResult.Location);
            StartInteract(OtherActor, InteractData);
        }
        // Manual trigger mode sets ready state
        else if (InteractMode == EInteractMode::ManualTrigger)
        {
            SetInteractState(EInteractState::Ready);
        }
    }
}

void AInterectItemBase::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValidInteractor(OtherActor))
    {
        // If leaving range while interacting, cancel interaction
        if (IsInteractingWithActor(OtherActor))
        {
            CancelInteract(OtherActor);
        }

        // If no other interactors, return to idle state
        if (CurrentInteractingActors.Num() == 0 && CurrentState == EInteractState::Ready)
        {
            SetInteractState(EInteractState::Idle);
        }
    }
}

bool AInterectItemBase::IsValidInteractor(AActor* Interactor) const
{
    if (!Interactor)
        return false;

    // Here you can add more complex validation logic
    // For example, check if it's a player, has specific components, etc.

    return true;
}

void AInterectItemBase::UpdateTriggerRange()
{
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(InteractRange);
    }
}