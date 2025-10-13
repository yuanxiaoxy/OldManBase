// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "InterectItemBase.generated.h"

// Interaction mode enum
UENUM(BlueprintType)
enum class EInteractMode : uint8
{
    AutoTrigger        UMETA(DisplayName = "Auto Trigger"),
    ManualTrigger      UMETA(DisplayName = "Manual Trigger"),
    MouseControl       UMETA(DisplayName = "Mouse Control"),
    Hybrid             UMETA(DisplayName = "Hybrid Mode")
};

// Interaction state enum
UENUM(BlueprintType)
enum class EInteractState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Ready              UMETA(DisplayName = "Ready"),
    Active             UMETA(DisplayName = "Active"),
    Completed          UMETA(DisplayName = "Completed"),
    Disabled           UMETA(DisplayName = "Disabled")
};

// Interaction data structure
USTRUCT(BlueprintType)
struct FInteractData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    AActor* InteractingActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    FVector InteractionPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    FVector InteractionDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    float InteractionValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
    TArray<FString> CustomData;

    FInteractData() {}

    FInteractData(AActor* InActor, const FVector& InPoint = FVector::ZeroVector)
        : InteractingActor(InActor), InteractionPoint(InPoint)
    {
    }

    FInteractData(AActor* InActor, float InValue)
        : InteractingActor(InActor), InteractionValue(InValue)
    {
    }
};

// Interaction delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractStateChanged, AActor*, InteractItem, EInteractState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractTriggered, AActor*, InteractItem, AActor*, InteractingActor, const FInteractData&, InteractData);

UCLASS(Abstract, Blueprintable, BlueprintType)
class XYFRAME_API AInterectItemBase : public AActor
{
    GENERATED_BODY()

public:
    AInterectItemBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

public:
    // ========== Basic Properties ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    FString InteractItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    EInteractMode InteractMode = EInteractMode::ManualTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact|Base")
    EInteractState CurrentState = EInteractState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    bool bIsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    float InteractRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact|Base")
    bool bAutoRegisterToManager = true;

    // ========== Components ==========

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact|Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact|Components")
    USceneComponent* SceneRoot;

    // ========== Event Delegates ==========

    UPROPERTY(BlueprintAssignable, Category = "Interact|Events")
    FOnInteractStateChanged OnInteractStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Interact|Events")
    FOnInteractTriggered OnInteractTriggered;

    // ========== Basic Methods ==========

    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual void SetInteractState(EInteractState NewState);

    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual void SetEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual bool CanInteract(AActor* InteractingActor) const;

    UFUNCTION(BlueprintCallable, Category = "Interact")
    virtual FText GetInteractPrompt() const;

    // ========== Core Interaction Methods ==========

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
    void StartInteract(AActor* InteractingActor, const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
    void EndInteract(AActor* InteractingActor);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
    void UpdateInteract(const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact")
    void CancelInteract(AActor* InteractingActor);

    // ========== Mouse Control Methods ==========

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact|Mouse")
    void StartMouseControl(const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact|Mouse")
    void UpdateMouseControl(const FInteractData& InteractData);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interact|Mouse")
    void EndMouseControl();

    // ========== Utility Methods ==========

    UFUNCTION(BlueprintCallable, Category = "Interact|Utility")
    float GetDistanceToActor(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Interact|Utility")
    bool IsInInteractRange(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Interact|Utility")
    TArray<AActor*> GetCurrentInteractingActors() const { return CurrentInteractingActors; }

    UFUNCTION(BlueprintCallable, Category = "Interact|Utility")
    bool IsInteractingWithActor(AActor* Actor) const;

protected:
    // ========== Override Methods ==========

    UFUNCTION()
    virtual void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    virtual bool IsValidInteractor(AActor* Interactor) const;

    virtual void UpdateTriggerRange();

private:
    UPROPERTY()
    TArray<AActor*> CurrentInteractingActors;

    // Default implementations
    virtual void StartInteract_Implementation(AActor* InteractingActor, const FInteractData& InteractData);
    virtual void EndInteract_Implementation(AActor* InteractingActor);
    virtual void UpdateInteract_Implementation(const FInteractData& InteractData);
    virtual void CancelInteract_Implementation(AActor* InteractingActor);
    virtual void StartMouseControl_Implementation(const FInteractData& InteractData);
    virtual void UpdateMouseControl_Implementation(const FInteractData& InteractData);
    virtual void EndMouseControl_Implementation();
};