// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/States/CableState/OldManOnCableState.h"
#include "OldManOnCableHorizontalJumpState.generated.h"

/**
 * State for horizontal jumping between cables.
 */
UCLASS()
class OLDMAN_API UOldManOnCableHorizontalJumpState : public UOldManOnCableState
{
    GENERATED_BODY()

public:
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update(float DeltaTime) override;

protected:
    virtual void SetupTransitionRules() override;

    /** Handles the horizontal jump logic */
    void HandleHorizontalJump(float DeltaTime);

    /** Returns the lateral jump speed */
    float GetLateralJumpSpeed();

private:
    /** Progress of the jump (0 to 1) */
    float JumpProgress = 0.0f;

    /** Start position of the jump */
    FVector JumpStartPosition;

    /** Target position of the jump */
    FVector JumpTargetPosition;

    /** Arc height for the jump (unused in current linear interpolation) */
    float ArcHeight;

    bool InLeftOfTarget(const FVector& Forward, const FVector& StartPos, const FVector& TargetPos);

};