// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "InputCoreTypes.h"     
#include "GameFramework/PlayerController.h"  
#include "Components/SphereComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ApproachEnemyCharacter.generated.h"

UCLASS()
class OLDMANENEMY_API AApproachEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AApproachEnemyCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    /*virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;*/

public:
    // 被鼠标点击时的处理
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void HandleOnClicked(AActor* TouchedActor, FKey ButtonPressed);

    // 设置屏幕位置（由管理器调用）
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void SetScreenPosition(const FVector2D& InScreenPos, float InCurrentDistance);

    // 获取点击判定半径
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    float GetClickRadius() const;

    // 检查鼠标是否在点击范围内
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    bool IsMouseOverlapping(const FVector2D& MousePosition) const;

    // 击杀敌人
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void KillEnemy();

    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void Recycle();

    // 初始化
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void InitializeEnemy(const FVector2D& InScreenPosition, float InitialDistance,
        float InAttackDistance, float InApproachSpeed);

    // 更新屏幕位置
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void UpdateScreenSpacePosition(float DeltaTime);

    // 屏幕坐标->世界坐标
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
     FVector GetWorldPositionFromScreen(const FVector2D& ScreenPos, float Distance);

    // 造成伤害
    UFUNCTION(BlueprintCallable, Category = "ApproachEnemy")
    void CheckAndApplyDamage();


public:
    // 可配置参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Settings")
    float BaseClickRadius = 50.0f;  // 基础点击半径（像素）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Settings")
    float MaxClickRadius = 200.0f;  // 最大点击半径

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Settings")
    float DeathEffectDuration = 0.5f;  // 死亡特效持续时间

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Settings")
    float DamagePerSecond = 10.0f;  // 每秒对玩家造成的伤害

private:
    // 组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* ClickCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* DebugSphere;  // 调试用，显示点击范围

    // 状态
    FVector2D ScreenPosition;  // 屏幕位置（0-1标准化坐标）
    float CurrentDistance;     // 当前距离摄像机距离
    float CurrentClickRadius;  // 当前点击半径

    bool bIsDead;
    float DeathTimer;

     UPROPERTY()
    APlayerController* PlayerController;
    
    UPROPERTY()
    APawn* PlayerPawn;
    float MaxDistance;
    float ApproachSpeed;
    float AttackDistance;

 


    // 动态材质用于视觉效果
    UPROPERTY()
    UMaterialInstanceDynamic* DynamicMaterial;

    // 更新点击判定大小
    void UpdateClickCollision();

    // 更新视觉效果
    void UpdateVisualEffects(float DeltaTime);



};
