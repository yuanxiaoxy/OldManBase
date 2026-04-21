#pragma once

#include "CoreMinimal.h"
#include "Character/States/CableState/OldManOnCableState.h"
#include "OldManOnCableMoveState.generated.h"

/**
 * State for moving along the cable.
 */
UCLASS()
class OLDMAN_API UOldManOnCableMoveState : public UOldManOnCableState
{
    GENERATED_BODY()

public:
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update(float DeltaTime) override;

protected:
    virtual void SetupTransitionRules() override;
    virtual void SetGravityScale() override;

    /** Updates movement along the cable */
    void UpdateCableMovement(float DeltaTime);

    /** Handles input for lateral cable detection */
    void HandleCableInput(float DeltaTime);

private:
    /** Current speed along the cable */
    float CurrentSpeed = 0.0f;

    /** Current input direction (lateral) */
    float CurrentInputDirection = 0.0f;

    FName MoveSoundName;
};