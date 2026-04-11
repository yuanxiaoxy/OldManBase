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
    // ===== 新增：在组件初始化后立即调用 =====
    virtual void PreInitializeComponents() override;

    virtual void BeginPlay() override;

    // 重写初始化方法
    virtual void LoadWorldResources_Implementation() override;
    virtual void InitializeWorldState_Implementation() override;
    virtual void InitializePlayers_Implementation() override;
    virtual APawn* SpawnPlayer_Implementation(AController* NewPlayer) override;

    // 事件处理
    UFUNCTION()
    void OnMyWorldInitialized(bool bSuccess);

    UFUNCTION()
    void OnUIInitialized(bool bSuccess);

    // UI相关
    UFUNCTION(BlueprintCallable, Category = "GameMode|UI")
    void ShowStartUI();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManWorldBase")
    bool IsStartWorld = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataAsset* UIConfig = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* EffectConfig = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* AudioConfig = nullptr;  
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* AudioEffectConfig = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* TaskConfig = nullptr;  

private:
    void SetupInputModeForUI();
    void SetupInputModeForGame();

    UPROPERTY()
    class UUIManager* UIManager;

    UPROPERTY()
    TArray<AActor*> WorldActors;

    FTimerHandle UIInitTimerHandle;
    bool bIsUIShown = false;
    bool bShouldSpawnPlayerLater = false;
    AController* PendingPlayerController = nullptr;
};