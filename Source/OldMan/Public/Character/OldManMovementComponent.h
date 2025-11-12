#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "OldManMovementComponent.generated.h"

UCLASS()
class OLDMAN_API UOldManMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()

public:
    UOldManMovementComponent();
};