// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/OldManGameMode.h"
#include "UObject/ConstructorHelpers.h"

AOldManGameMode::AOldManGameMode()
{
	OnWorldInitialized.AddDynamic(this, &AOldManGameMode::OnMyWorldInitialized);
}

void AOldManGameMode::LoadWorldResources_Implementation()
{
    Super::LoadWorldResources_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Loading my specific world resources..."));
}

void AOldManGameMode::InitializeWorldState_Implementation()
{
    Super::InitializeWorldState_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Initializing my specific world state..."));

    // ��ʼ��NPC������ϵͳ��
    // ...
}

void AOldManGameMode::InitializePlayers_Implementation()
{
    Super::InitializePlayers_Implementation();

    UE_LOG(LogTemp, Log, TEXT("Initializing my specific players..."));

    // �Զ�����ҳ�ʼ���߼�
    // ...
}

FWorldConfig AOldManGameMode::GetWorldConfig_Implementation() const
{
    FWorldConfig Config = Super::GetWorldConfig_Implementation();

    // �Զ�������
    Config.WorldName = "My Custom World";
    Config.WorldDescription = "This is my custom world implementation";
    Config.bLoadFromSave = true;
    Config.SaveSlotName = "MyWorldSave";
    Config.bAsyncInitialization = true;
    Config.InitializationDelay = 2.0f;

    return Config;
}

FPlayerSpawnConfig AOldManGameMode::GetPlayerSpawnConfig_Implementation() const
{
    FPlayerSpawnConfig Config = Super::GetPlayerSpawnConfig_Implementation();

    // �Զ��������������
    Config.bShouldSpawnPlayer = true;
    Config.bUseRandomSpawn = true;

    // ��ӿ��ܵ����ɵ�
    Config.PossibleSpawnPoints.Add(FTransform(FRotator(0, 0, 0), FVector(100, 200, 300)));
    Config.PossibleSpawnPoints.Add(FTransform(FRotator(0, 90, 0), FVector(-100, 150, 300)));
    Config.PossibleSpawnPoints.Add(FTransform(FRotator(0, 180, 0), FVector(50, -200, 300)));

    return Config;
}

void AOldManGameMode::OnMyWorldInitialized(bool bSuccess)
{
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("My world initialized successfully!"));

        // �����ʼ���ɹ���ִ�еĲ���
        // ���翪ʼ��Ϸ�߼�����ʾUI��
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("My world initialization failed!"));
    }
}