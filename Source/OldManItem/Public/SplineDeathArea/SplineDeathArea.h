// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "SplineDeathArea.generated.h"

// 声明动态多播委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeathAreaEvent, APawn*, PlayerPawn);

UCLASS(Blueprintable, BlueprintType)
class OLDMANITEM_API ASplineDeathArea : public AActor
{
    GENERATED_BODY()

//public:
//    ASplineDeathArea();
//
//    // --- 配置参数 ---
//
//    /* 基础碰撞半径 */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area")
//    float Radius = 50.f;
//
//    /* 每米多少段碰撞体 (值越高越平滑，但更消耗性能) */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area", meta = (ClampMin = 0.05f))
//    float SegmentsPerMeter = 1.0f;
//
//    /* 编辑器实时更新碰撞 */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area")
//    bool bLiveUpdate = true;
//
//    /* 是否显示调试球体 (仅在编辑器中显示) */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area|Debug")
//    bool bShowDebugVisualization = true;
//
//    /* 调试球体颜色 */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area|Debug")
//    FColor DebugColor = FColor::Red;
//
//    /* 节点半径数组 */
//    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death Area")
//    TArray<float> NodeRadii;
//
//    // --- 事件 ---
//
//    /* 当玩家进入区域时触发 */
//    UPROPERTY(BlueprintAssignable, Category = "Death Area")
//    FOnPlayerDeathAreaEvent OnPlayerEnterDeathArea;
//
//    /* 当玩家离开区域时触发 (通常用于安全区离开即死) */
//    UPROPERTY(BlueprintAssignable, Category = "Death Area")
//    FOnPlayerDeathAreaEvent OnPlayerExitDeathArea;
//
//protected:
//    virtual void OnConstruction(const FTransform& Transform) override;
//    virtual void BeginPlay() override;
//    virtual void Tick(float DeltaTime) override;
//
//private:
//    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
//    USplineComponent* Spline;
//
//    UPROPERTY()
//    TArray<class UCapsuleComponent*> CollisionCapsules;
//
//    // 核心逻辑：重建碰撞
//    void RebuildCollision();
//
//    // 工具函数：获取插值半径
//    float GetRadiusAtDistance(float Distance) const;
//
//    // 碰撞回调
//    UFUNCTION()
//    void OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
//
//    UFUNCTION()
//    void OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
//
//#if WITH_EDITOR
//    virtual void PostEditMove(bool bFinished) override;
//
//    // 【新增】立即绘制调试图形的函数
//    void DrawDebugVisualization_Now();
//
//    // 旧的空函数声明（保留以确保兼容性）
//    void DrawDebugVisualization() const;
//#endif
};