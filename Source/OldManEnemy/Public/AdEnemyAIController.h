// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AdEnemyAIController.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class OLDMANENEMY_API AAdEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    AAdEnemyAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI组件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBlackboardComponent* BlackboardComponent;

public:
    // 获取Blackboard
    UBlackboardComponent* GetBlackboard() const { return BlackboardComponent; }

    // 设置目标玩家
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetPlayer(AActor* PlayerActor);

    // 通知其他怪物
    UFUNCTION(BlueprintCallable, Category = "AI")
    void NotifyOtherMonsters(AActor* SpottedPlayer);

	
};


