// DraggableSplineActorManager.h
#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/ActorSingletonBase.h"
#include "DraggableSplineActor/DraggableSplineActor.h"
#include "DraggableSplineActorManager.generated.h"

// 聚合的拖动事件委托，与 Actor 自身的事件签名保持一致
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyDraggingStarted, ADraggableSplineActor*, DraggedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyDraggingStopped, ADraggableSplineActor*, DraggedActor);

// 通用分组数据结构（保持不变）
USTRUCT(BlueprintType)
struct FDraggableItemData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggableActorManager")
    TArray<ADraggableSplineActor*> DraggableSplineActors;
};

/**
 * 可拖动物体管理器（单例 Actor）
 * 自动监听所有 ADraggableSplineActor 的拖动事件，并提供分组管理和全局事件广播。
 */
UCLASS(Blueprintable)
class OLDMANITEM_API ADraggableSplineActorManager : public AActorSingletonBase
{
    GENERATED_BODY()

    DECLARE_ACTOR_SINGLETON(ADraggableSplineActorManager)

public:
    ADraggableSplineActorManager();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DraggableSplineActorManager", meta = (DisplayName = "Get DraggableSplineActorManager"))
    static ADraggableSplineActorManager* GetDraggableSplineActorManager() { return GetInstance(); }

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ----- 分组管理 -----
    /** 用于编辑器中配置的分组映射 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DraggableActorManager")
    TMap<FString, FDraggableItemData> DraggableActorMap;

    /** 重置指定分组中所有物体的位置到起始点 */
    UFUNCTION(BlueprintCallable, Category = "DraggableActorManager")
    void ResetDraggableSplineActorPos(const FString& GroupName);

    // ----- 动态注册 / 注销（支持运行时生成的 Actor）-----
    /** 手动注册一个可拖动物体（动态生成的 Actor 不会自动绑定，需调用此函数） */
    UFUNCTION(BlueprintCallable, Category = "DraggableActorManager")
    void RegisterDraggableActor(ADraggableSplineActor* Actor);

    /** 手动注销一个可拖动物体 */
    UFUNCTION(BlueprintCallable, Category = "DraggableActorManager")
    void UnregisterDraggableActor(ADraggableSplineActor* Actor);

    // ----- 查询接口 -----
    /** 获取当前所有受管理的 Actor（弱引用列表） */
    UFUNCTION(BlueprintPure, Category = "DraggableActorManager")
    TArray<ADraggableSplineActor*> GetAllManagedActors() const;

    // ----- 全局拖动事件（供外部系统如任务系统监听）-----
    /** 任意受管理的物体开始拖动时广播 */
    UPROPERTY(BlueprintAssignable, Category = "DraggableActorManager")
    FOnAnyDraggingStarted OnAnyDraggingStarted;

    /** 任意受管理的物体停止拖动时广播 */
    UPROPERTY(BlueprintAssignable, Category = "DraggableActorManager")
    FOnAnyDraggingStopped OnAnyDraggingStopped;

private:
    /** 受管理的 Actor 集合（使用弱指针防止悬空） */
    UPROPERTY()
    TSet<TWeakObjectPtr<ADraggableSplineActor>> ManagedActors;

    /** 绑定单个 Actor 的委托 */
    void BindActorEvents(ADraggableSplineActor* Actor);

    /** 解绑单个 Actor 的委托 */
    void UnbindActorEvents(ADraggableSplineActor* Actor);

    /** 扫描场景中已有的所有 ADraggableSplineActor 并注册 */
    void DiscoverExistingActors();

    // 委托回调
    UFUNCTION()
    void HandleDraggingStarted(ADraggableSplineActor* DraggedActor);

    UFUNCTION()
    void HandleDraggingStopped(ADraggableSplineActor* DraggedActor);
};