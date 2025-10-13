// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "InterectItemBase.h"
#include "InterectItemManager.generated.h"

// Interact item filter criteria
USTRUCT(BlueprintType)
struct FInteractFilter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractFilter")
    TSubclassOf<AInterectItemBase> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractFilter")
    float MaxDistance = -1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractFilter")
    EInteractState RequiredState = EInteractState::Ready;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractFilter")
    bool bEnabledOnly = true;

    FInteractFilter() {}

    FInteractFilter(TSubclassOf<AInterectItemBase> InClass, float InMaxDistance = -1.0f)
        : ItemClass(InClass), MaxDistance(InMaxDistance)
    {
    }
};

// Interact manager statistics
USTRUCT(BlueprintType)
struct FInteractManagerStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractStats")
    int32 TotalItems = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractStats")
    int32 EnabledItems = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractStats")
    int32 ActiveInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractStats")
    TMap<EInteractState, int32> ItemsByState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InteractStats")
    TMap<EInteractMode, int32> ItemsByMode;

    FInteractManagerStats()
    {
        for (int32 i = 0; i < static_cast<int32>(EInteractState::Disabled) + 1; i++)
        {
            ItemsByState.Add(static_cast<EInteractState>(i), 0);
        }
        for (int32 i = 0; i < static_cast<int32>(EInteractMode::Hybrid) + 1; i++)
        {
            ItemsByMode.Add(static_cast<EInteractMode>(i), 0);
        }
    }
};

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UInterectItemManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UInterectItemManager)

public:
    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void InitializeInteractManager();

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "InteractManager", meta = (DisplayName = "Get Interact Manager"))
    static UInterectItemManager* GetInteractManager() { return GetInstance(); }

    UInterectItemManager();
    virtual ~UInterectItemManager() override;

    // ========== Interact Item Registration Management ==========

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void RegisterInteractItem(AInterectItemBase* InteractItem);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void UnregisterInteractItem(AInterectItemBase* InteractItem);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    AInterectItemBase* FindInteractItemById(const FString& ItemId) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    AInterectItemBase* FindInteractItemByTag(FName Tag) const;

    // ========== Interact Item Queries ==========

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetAllInteractItems() const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetInteractItemsByClass(TSubclassOf<AInterectItemBase> ItemClass) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetInteractItemsByState(EInteractState State) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetInteractItemsByMode(EInteractMode Mode) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    AInterectItemBase* GetNearestInteractItem(const FVector& Location, const FInteractFilter& Filter = FInteractFilter()) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetInteractItemsNearActor(AActor* Actor, float Radius = 500.0f, const FInteractFilter& Filter = FInteractFilter()) const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    TArray<AInterectItemBase*> GetAvailableInteractItemsForActor(AActor* Actor) const;

    // ========== Interaction Control ==========

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    bool StartInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData = FInteractData());

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    bool EndInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    bool UpdateInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    bool CancelInteraction(AActor* InteractingActor, AInterectItemBase* InteractItem);

    UFUNCTION(BlueprintCallable, Category = "InteractManager|Mouse")
    bool StartMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData = FInteractData());

    UFUNCTION(BlueprintCallable, Category = "InteractManager|Mouse")
    bool UpdateMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem, const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, Category = "InteractManager|Mouse")
    bool EndMouseControl(AActor* InteractingActor, AInterectItemBase* InteractItem);

    // ========== Batch Operations ==========

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void SetAllInteractItemsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void SetInteractItemsEnabledByClass(TSubclassOf<AInterectItemBase> ItemClass, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void ResetAllInteractItems();

    // ========== Statistics and Debugging ==========

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    FInteractManagerStats GetStats() const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void PrintAllInteractItems() const;

    UFUNCTION(BlueprintCallable, Category = "InteractManager")
    void PrintStats() const;

    // ========== Event Handling ==========

    UFUNCTION()
    void OnInteractItemStateChanged(AActor* InteractItem, EInteractState NewState);

    UFUNCTION()
    void OnInteractItemTriggered(AActor* InteractItem, AActor* InteractingActor, const FInteractData& InteractData);

private:
    UPROPERTY()
    TArray<AInterectItemBase*> RegisteredInteractItems;

    TMap<FString, AInterectItemBase*> InteractItemsById;
    TMap<TSubclassOf<AInterectItemBase>, TArray<AInterectItemBase*>> InteractItemsByClass;
    TMap<AActor*, TArray<AInterectItemBase*>> ActiveInteractions;

    void UpdateItemMappings(AInterectItemBase* InteractItem, bool bAdd);
    bool IsValidForFilter(AInterectItemBase* Item, const FInteractFilter& Filter, const FVector* QueryLocation = nullptr) const;

    UWorld* GetWorld() const override;
};