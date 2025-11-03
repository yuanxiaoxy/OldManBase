#pragma once

#include "CoreMinimal.h"
#include "Character/States/OldManStateBase.h"
#include "OldManFallingState.generated.h"

UCLASS()
class OLDMAN_API UOldManFallingState : public UOldManStateBase
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;
	virtual void Update(float DeltaTime) override;

protected:
	virtual void SetupTransitionRules() override;
};