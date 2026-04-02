// DeathSafeAreaManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OctreeDeathArea/DeathSafeAreaTypes.h"
#include "DeathSafeAreaManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAreaStateChanged, AActor*, Actor, bool, bIsSafe);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OLDMANITEM_API UDeathSafeAreaManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDeathSafeAreaManagerComponent();

    // ========== 配置 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Octree")
    FBox WorldBounds = FBox(FVector(-10000), FVector(10000));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Octree", meta = (ClampMin = 1, ClampMax = 8))
    int32 MaxDepth = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Octree")
    EOctreeMode Mode = EOctreeMode::DefaultSafe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking")
    float MovementThreshold = 50.0f;

    // 是否启用调试绘制（显示所有区域边界框）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebug = true;

    // ========== 事件 ==========
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAreaStateChanged OnAreaStateChanged;

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void AddDeathArea(const FBox& Bounds, const FColor& DebugColor = FColor::Red, const FString& AreaName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void AddSafeArea(const FBox& Bounds, const FColor& DebugColor = FColor::Green, const FString& AreaName = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void RemoveArea(const FBox& Bounds);

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void ClearAllAreas();

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void RebuildOctree();

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    void UpdateActorLocation(AActor* Actor, const FVector& NewLocation);

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    bool IsLocationSafe(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    FAreaQueryResult QueryLocation(const FVector& Location) const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    void RebuildOctreeInternal();
    bool IsLocationInDeathArea(const FVector& Location) const;
    bool IsLocationInSafeArea(const FVector& Location) const;
    bool HasMovedBeyondThreshold(const FVector& OldLocation, const FVector& NewLocation) const;

private:
    TArray<FAreaElement> DeathAreas;   // 死亡区域列表
    TArray<FAreaElement> SafeAreas;    // 安全区域列表
    TUniquePtr<FAreaOctree> Octree;    // 八叉树（用于查询）

    TMap<AActor*, FVector> LastKnownLocations;
    TMap<AActor*, bool> LastKnownSafeState;

    mutable FRWLock OctreeLock;

    void AddAreaToList(const FBox& Bounds, const FColor& Color, const FString& Name, TArray<FAreaElement>& TargetList);
};