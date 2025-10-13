// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "XyFrame/Public/XyGameModeBase/XyBaseGameMode.h"
#include "OldManGameMode.generated.h"

UCLASS(minimalapi)
class AOldManGameMode : public AXyBaseGameMode
{
	GENERATED_BODY()

public:
	AOldManGameMode();

protected:
    // ��д��ʼ������
    virtual void LoadWorldResources_Implementation() override;
    virtual void InitializeWorldState_Implementation() override;
    virtual void InitializePlayers_Implementation() override;

    // ��д���÷���
    virtual FWorldConfig GetWorldConfig_Implementation() const override;
    virtual FPlayerSpawnConfig GetPlayerSpawnConfig_Implementation() const override;

    // �¼�����
    UFUNCTION()
    void OnMyWorldInitialized(bool bSuccess);

private:
    UPROPERTY()
    TArray<AActor*> WorldActors;
};



