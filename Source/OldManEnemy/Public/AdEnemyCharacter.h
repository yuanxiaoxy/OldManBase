// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "AdEnemyStateTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FEnemyLocationInfo.h"
#include "ObjectPool/ObjectPoolManager.h"
#include "EnemyInitializationInterface.h"
#include "AdEnemyCharacter.generated.h"



// 前向声明放在这里
class UBehaviorTree;
class AOldManCharacter;
class AEnemyPatrolPoint;
class AAdEnemyAIController;

UCLASS(Blueprintable)
class OLDMANENEMY_API AAdEnemyCharacter : public ACharacter, public IObjectPoolInterface, public IEnemyInitializationInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AAdEnemyCharacter();
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;


protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;



public:

    virtual void OnSpawn_Implementation() override;
    virtual void OnDespawn_Implementation() override;

    // Called every frame
    virtual void Tick(float DeltaTime) override;


    // 状态变量
    UPROPERTY(BlueprintReadWrite, Category = "AI|State")
    EAdMonsterState CurrentState = EAdMonsterState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdEnmey")
    TArray<AEnemyPatrolPoint*> Path;

    // 基础数据
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Health = 1;

    UPROPERTY(BlueprintReadOnly, Category = "AdEnmey")
    int32 CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "AdEnmey")
    bool bIsDead;

    UPROPERTY(BlueprintReadOnly, Category = "AdEnmey")
    AAdEnemyAIController* AIController;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 AttackPower = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AOldManCharacter* Player;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float DetectRadius = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
    float AttackRadius = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // 核心功能函数
    /*UFUNCTION(BlueprintCallable, Category = "AI")*/
    UFUNCTION()
    void ChangeState(EAdMonsterState NewState);

    // 圆锥攻击检测
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool DectectPlayer();
    // 激光攻击
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformLaserAttack();
    // 受到多少伤害
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(int32 DamageAmount);

    virtual void InitializeEnemy_Implementation(const FEnemyLocationInfo& EnemyInfo) override;
};



