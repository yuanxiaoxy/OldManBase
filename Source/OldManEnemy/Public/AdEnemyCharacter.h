// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "AdEnemyStateTypes.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AdEnemyCharacter.generated.h"



// 前向声明放在这里
class UBehaviorTree;


UCLASS(Blueprintable)
class OLDMANENEMY_API AAdEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAdEnemyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


    // 状态变量
    UPROPERTY(BlueprintReadWrite, Category = "AI|State")
    EAdMonsterState CurrentState = EAdMonsterState::Patrol;

    // 基础数据
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 Health = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 AttackPower = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
    AActor* StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // 核心功能函数
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ChangeState(EAdMonsterState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetNextPatrolPosition();
    // 圆锥攻击检测
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool PerformConeAttack();  
    // 激光攻击
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformLaserAttack();  
    // 受到多少伤害
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TakeDamage(int32 DamageAmount);


private:
    AActor* _currentPoint;
};



