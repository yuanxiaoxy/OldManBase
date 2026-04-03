#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OctreeDeathArea/DeathSafeAreaManagerComponent.h"
#include "SingletonBase/ActorSingletonBase.h"
#include "DeathSafeAreaManager.generated.h"

/**
 * 负责管理死亡/安全区域的 Actor，方便在关卡中放置。
 * 内部包含 UDeathSafeAreaManagerComponent 组件，并提供蓝图可调用的封装函数。
 */
UCLASS(BlueprintType, Blueprintable, meta = (ShowCategories = "DeathSafeArea"))
class OLDMANITEM_API ADeathSafeAreaManager : public AActorSingletonBase
{
    GENERATED_BODY()

    DECLARE_ACTOR_SINGLETON(ADeathSafeAreaManager);

public:
    ADeathSafeAreaManager();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DeathSafeArea", meta = (DisplayName = "Get DeathSafeArea Manager"))
    static ADeathSafeAreaManager* GetDeathSafeAreaManager() { return GetInstance(); }

    // 获取内部的管理器组件（供 C++ 调用）
    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    UDeathSafeAreaManagerComponent* GetManagerComponent() const { return ManagerComponent; }

    // 以下函数直接转发给组件，方便蓝图使用
    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void AddDeathArea(const FBox& Bounds, const FColor& DebugColor = FColor::Red, const FString& AreaName = TEXT(""))
    {
        if (ManagerComponent) ManagerComponent->AddDeathArea(Bounds, DebugColor, AreaName);
    }

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void AddSafeArea(const FBox& Bounds, const FColor& DebugColor = FColor::Green, const FString& AreaName = TEXT(""))
    {
        if (ManagerComponent) ManagerComponent->AddSafeArea(Bounds, DebugColor, AreaName);
    }

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void RemoveArea(const FBox& Bounds)
    {
        if (ManagerComponent) ManagerComponent->RemoveArea(Bounds);
    }

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void ClearAllAreas()
    {
        if (ManagerComponent) ManagerComponent->ClearAllAreas();
    }

    UFUNCTION(BlueprintCallable, Category = "DeathSafeArea")
    void RebuildOctree()
    {
        if (ManagerComponent) ManagerComponent->RebuildOctree();
    }

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    void UpdateActorLocation(AActor* Actor, const FVector& NewLocation)
    {
        if (ManagerComponent) ManagerComponent->UpdateActorLocation(Actor, NewLocation);
    }

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    bool IsLocationSafe(const FVector& Location) const
    {
        return ManagerComponent ? ManagerComponent->IsLocationSafe(Location) : true;
    }

    UFUNCTION(BlueprintCallable, Category = "Tracking")
    FAreaQueryResult QueryLocation(const FVector& Location) const
    {
        return ManagerComponent ? ManagerComponent->QueryLocation(Location) : FAreaQueryResult();
    }

    // 暴露事件给蓝图
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAreaStateChanged OnAreaStateChanged;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UDeathSafeAreaManagerComponent* ManagerComponent;

    UFUNCTION()
    void OnManagerStateChanged(AActor* Actor, bool bIsSafe);
};