// Fill out your copyright notice in the Description page of Project Settings.


#include "Task/Task_CheckPlayerAction.h"

void UTask_CheckPlayerAction::InitializeTask(const FTaskConfigRow& ConfigRow)
{
    Super::InitializeTask(ConfigRow);
}

AOldManCharacter* UTask_CheckPlayerAction::GetCachedPlayer()
{
    if (!Cachedlayer)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            APlayerController* PC = World->GetFirstPlayerController();
            APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
            // Cache character pointer
            Cachedlayer = Cast<AOldManCharacter>(PlayerPawn);
        }
    }

    return Cachedlayer;
}
