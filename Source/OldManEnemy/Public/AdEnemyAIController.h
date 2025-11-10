// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AdEnemyAIController.generated.h"

/**
 * 
 */
class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAdEnemyCharacter;


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
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY()
    AAdEnemyCharacter* EnemyCharacter;

public:
    // 获取Blackboard
    UBlackboardComponent* GetBlackboard() const { return BlackboardComponent; }

    // 设置目标玩家
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetTargetPlayer(AActor* PlayerActor);

    // 通知其他怪物
    UFUNCTION(BlueprintCallable, Category = "AI")
    void NotifyOtherMonsters(AActor* SpottedPlayer);

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsPlayerDetected(float radius);
	
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ChangeState(EAdMonsterState state);

};


