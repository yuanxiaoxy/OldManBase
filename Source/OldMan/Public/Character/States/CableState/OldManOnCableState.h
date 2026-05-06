#pragma once

#include "CoreMinimal.h"
#include "Character/States/OldManStateBase.h"
#include "ItemBase/OldManCableBase.h"
#include "OldManOnCableState.generated.h"

/** Structure to hold cable detection results */
USTRUCT(BlueprintType)
struct FCableDetectionResult
{
    GENERATED_BODY()

    /** The detected cable */
    UPROPERTY()
    AOldManCableBase* Cable = nullptr;

    /** Position on the cable (adjusted for character) */
    UPROPERTY()
    FVector Position;

    /** Distance from character to cable position */
    UPROPERTY()
    float Distance = 0.0f;

    /** Distance along the cable spline */
    UPROPERTY()
    float CableDistance = 0.0f;
};

/**
 * Base state for being on a cable.
 * Handles cable attachment, gravity, and nearby cable detection.
 */
UCLASS()
class OLDMAN_API UOldManOnCableState : public UOldManStateBase
{
    GENERATED_BODY()

public:
    virtual void Enter() override;
    virtual void Update(float DeltaTime) override;
    virtual void Exit() override;

protected:
    /** Updates detection of nearby cables in the given horizontal direction */
    void UpdateNearbyCableDetection(float HorizontalDir);

    /** Finds a cable within a box oriented along the character's local axes */
    FCableDetectionResult FindCableInBox(const FVector& Direction, float Width, float Height, float Length);

    // Cable properties - accessible by child states
    /** Current cable the character is on */
    UPROPERTY()
    AOldManCableBase* CurrentCable;

    /** Current distance along the cable spline */
    UPROPERTY()
    float CurrentCableDistance = 0.0f;

    /** Lateral jump distance (horizontal offset) */
    UPROPERTY()
    float LateralJumpDistance = 0.0f;

    /** Length of detection box for lateral jumps */
    UPROPERTY()
    float DetectionLength = 0.0f;

    /** Height of detection box for lateral jumps */
    UPROPERTY()
    float DetectionHeight = 0.0f;

    // Nearby cable detection results
    /** Cable detected on the left */
    UPROPERTY()
    FCableDetectionResult LeftCable;

    /** Cable detected on the right */
    UPROPERTY()
    FCableDetectionResult RightCable;

    // Helper methods for child states
    /** Calculates character's position on the cable considering capsule radius */
    FVector CalculateCharacterPositionOnCable(const FVector& WorldPosition);

    /** Aligns character rotation with the cable tangent */
    void AlignCharacterWithCable(const FVector& WorldPosition);

    /** Handles movement on cable when in air (unused in this context) */
    UFUNCTION()
    void HandleMovementOnCableInAir(float DeltaTime);

    /** Sets gravity scale for cable state */
    virtual void SetGravityScale() override;

    /** Sets player's current action state */
    virtual void SetPlayerCurActionState() override;

    /** Applies custom gravity along cable normal */
    void ApplyCableGravity(float DeltaTime);

    /** Sets up transition rules for this state */
    virtual void SetupTransitionRules() override;

    /** 自动计算在当前电缆上的移动方向 */
    void AutoDetermineCableDirection();

    UPROPERTY()
    FName Line_Name = FName();
    UPROPERTY()
    FName Sphere_Name = FName();

    UFUNCTION()
    void HideDetectionEffects();
};