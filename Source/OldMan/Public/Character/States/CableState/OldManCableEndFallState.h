#pragma once

#include "CoreMinimal.h"
#include "Character/States/OldManFallingState.h"
#include "OldManCableEndFallState.generated.h"

/**
 * State that handles falling from the end of a cable with an initial forward velocity.
 */
UCLASS()
class OLDMAN_API UOldManCableEndFallState : public UOldManFallingState
{
    GENERATED_BODY()

public:
    virtual void Enter() override;

private:
    /** Applies the launch velocity when entering the state */
    void ApplyLaunchVelocity();
};