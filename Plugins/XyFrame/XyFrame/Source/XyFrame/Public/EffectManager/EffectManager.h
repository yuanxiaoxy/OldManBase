// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/StreamableManager.h"
#include "EffectInfo.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "EffectManager.generated.h"

// 特效实例信息结构
USTRUCT(BlueprintType)
struct FEffectInstanceInfo
{
    GENERATED_BODY()

    // 实例ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FName InstanceID;

    // 特效ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FName EffectID;

    // Niagara特效组件（GC保护）
    UPROPERTY()
    UNiagaraComponent* EffectComponent = nullptr;

    // 父级Actor（跟随或附加特效使用）(GC保护)
    UPROPERTY()
    AActor* ParentActor = nullptr;

    // 临时Actor（仅世界空间特效使用）(GC保护)
    UPROPERTY()
    AActor* TempActor = nullptr;

    // 开始播放时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float StartTime = 0.0f;

    // 当前播放时间
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float CurrentTime = 0.0f;

    // 剩余播放次数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    int32 RemainingLoops = 0;

    // 剩余生命周期
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    float RemainingLifetime = 0.0f;

    // 是否正在播放
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bIsPlaying = false;

    // 是否已激活
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bIsActive = false;

    // 是否是手动跟随模式（而非真正的附加）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bIsFollowMode = false;

    // 特效配置信息
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    FEffectTableRow EffectConfig;

    // ========== 新增：自动跟随位置目标 ==========
    // 弱引用目标Actor，如果有效则每帧将特效位置设置为该Actor位置+偏移
    TWeakObjectPtr<AActor> FollowLocationTarget;
    // 跟随位置时的偏移量（世界空间偏移，不受目标旋转影响）
    FVector FollowLocationOffset;

    FEffectInstanceInfo()
        : EffectComponent(nullptr)
        , ParentActor(nullptr)
        , TempActor(nullptr)
        , StartTime(0.0f)
        , CurrentTime(0.0f)
        , RemainingLoops(0)
        , RemainingLifetime(0.0f)
        , bIsPlaying(false)
        , bIsActive(false)
        , bIsFollowMode(false)
        , FollowLocationOffset(FVector::ZeroVector)
    {
    }
};

// 特效状态枚举
UENUM(BlueprintType)
enum class EEffectInstanceState : uint8
{
    Created UMETA(DisplayName = "Created"),         // 已创建
    Delayed UMETA(DisplayName = "Delayed"),         // 延迟等待中
    Active UMETA(DisplayName = "Active"),           // 激活播放中
    Finished UMETA(DisplayName = "Finished"),       // 播放完成
    Destroyed UMETA(DisplayName = "Destroyed")      // 已销毁
};

// 特效播放完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectFinishedSignature, const FName&, InstanceID, const FName&, EffectID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectDestroyedSignature, const FName&, InstanceID, const FName&, EffectID);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnEffectFinishedCallback, const FName&, InstanceID, const FName&, EffectID);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnEffectDestroyedCallback, const FName&, InstanceID, const FName&, EffectID);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UEffectManager : public USingletonBase
{
    GENERATED_BODY()

    // 声明单例
    DECLARE_SINGLETON(UEffectManager)

public:
    // 构造函数
    UEffectManager();

    // 析构函数
    virtual ~UEffectManager() override;

    // 初始化特效管理器
    UFUNCTION(BlueprintCallable, Category = "Effect")
    void InitializeEffectManager();

    // 重写初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override;

    // 获取实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Effect", meta = (DisplayName = "Get Effect Manager"))
    static UEffectManager* GetEffectManager() { return GetInstance(); }

    // ========== 更新控制 ==========

    // 开始更新特效管理器（手动控制更新）
    UFUNCTION(BlueprintCallable, Category = "Effect|Update")
    void StartUpdating(float Interval = 0.1f);

    // 停止更新特效管理器
    UFUNCTION(BlueprintCallable, Category = "Effect|Update")
    void StopUpdating();

    // 手动更新特效管理器
    UFUNCTION(BlueprintCallable, Category = "Effect|Update")
    void UpdateEffectManager(float DeltaTime);

    // 获取是否正在更新
    UFUNCTION(BlueprintCallable, Category = "Effect|Update")
    bool IsUpdating() const { return bIsUpdating; }

    // ========== 蓝图可分配委托 ==========

    // 特效播放完成委托
    UPROPERTY(BlueprintAssignable, Category = "Effect")
    FOnEffectFinishedSignature OnEffectFinished;

    // 特效销毁委托
    UPROPERTY(BlueprintAssignable, Category = "Effect")
    FOnEffectDestroyedSignature OnEffectDestroyed;

    // ========== 数据表相关操作 ==========

    // 设置特效数据表
    UFUNCTION(BlueprintCallable, Category = "Effect|DataTable")
    void SetEffectDataTable(UDataTable* InEffectDataTable);

    // 获取特效配置信息
    UFUNCTION(BlueprintCallable, Category = "Effect|DataTable")
    bool GetEffectConfig(const FName& EffectID, FEffectTableRow& OutConfig) const;

    // 检查特效ID是否存在
    UFUNCTION(BlueprintCallable, Category = "Effect|DataTable")
    bool DoesEffectIDExist(const FName& EffectID) const;

    // ========== 特效播放控制 ==========

    // 播放特效（世界空间位置）
    UFUNCTION(BlueprintCallable, Category = "Effect")
    FName PlayEffectAtLocation(const FName& EffectID, const FVector& Location,
        const FRotator& Rotation = FRotator::ZeroRotator);

    // 播放特效（附加到Actor）
    UFUNCTION(BlueprintCallable, Category = "Effect")
    FName PlayEffectAttached(const FName& EffectID, AActor* TargetActor,
        FName SocketName = NAME_None,
        const FVector& RelativeOffset = FVector::ZeroVector,
        const FRotator& RelativeRotation = FRotator::ZeroRotator);

    // 播放特效（跟随Actor但不附加）
    UFUNCTION(BlueprintCallable, Category = "Effect")
    FName PlayEffectFollowActor(const FName& EffectID, AActor* TargetActor);

    // ========== 特效实例控制 ==========

    // 获取特效实例状态
    UFUNCTION(BlueprintCallable, Category = "Effect")
    EEffectInstanceState GetEffectInstanceState(const FName& InstanceID) const;

    // 激活特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect")
    void ActivateEffectInstance(const FName& InstanceID);

    // 停止特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect")
    void StopEffectInstance(const FName& InstanceID, bool bImmediate = true);

    // 销毁特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect")
    void DestroyEffectInstance(const FName& InstanceID);

    // 重启特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect")
    void RestartEffectInstance(const FName& InstanceID);

    // 检查特效实例是否有效（组件未销毁且未被标记待删除）
    UFUNCTION(BlueprintCallable, Category = "Effect")
    bool IsEffectInstanceValid(const FName& InstanceID) const;

    // ========== 特效实例位置/变换控制（新增） ==========

    // 手动设置特效实例的世界位置（适用于外部每帧更新位置）
    UFUNCTION(BlueprintCallable, Category = "Effect|Transform")
    void SetEffectWorldLocation(const FName& InstanceID, const FVector& NewLocation);

    // 手动设置特效实例的世界旋转
    UFUNCTION(BlueprintCallable, Category = "Effect|Transform")
    void SetEffectWorldRotation(const FName& InstanceID, const FRotator& NewRotation);

    // 手动设置特效实例的完整世界变换
    UFUNCTION(BlueprintCallable, Category = "Effect|Transform")
    void SetEffectWorldTransform(const FName& InstanceID, const FTransform& NewTransform);

    // 设置特效实例自动跟随某个Actor的位置（每帧自动更新，无需手动Tick）
    UFUNCTION(BlueprintCallable, Category = "Effect|Transform")
    void SetEffectFollowLocationTarget(const FName& InstanceID, AActor* TargetActor, FVector Offset = FVector::ZeroVector);

    // 清除特效实例的自动跟随设置
    UFUNCTION(BlueprintCallable, Category = "Effect|Transform")
    void ClearEffectFollowLocationTarget(const FName& InstanceID);

    // ========== 特效实例参数控制 ==========

    // 设置特效实例参数（Float）
    UFUNCTION(BlueprintCallable, Category = "Effect|Parameters")
    void SetEffectFloatParameter(const FName& InstanceID, const FName& ParameterName, float Value);

    // 设置特效实例参数（Vector）
    UFUNCTION(BlueprintCallable, Category = "Effect|Parameters")
    void SetEffectVectorParameter(const FName& InstanceID, const FName& ParameterName, const FVector& Value);

    // 设置特效实例参数（Color）
    UFUNCTION(BlueprintCallable, Category = "Effect|Parameters")
    void SetEffectColorParameter(const FName& InstanceID, const FName& ParameterName, const FLinearColor& Value);

    // 设置特效实例颜色（全局覆盖）
    UFUNCTION(BlueprintCallable, Category = "Effect|Parameters")
    void SetEffectInstanceColor(const FName& InstanceID, const FLinearColor& Color);

    // 设置特效实例大小
    UFUNCTION(BlueprintCallable, Category = "Effect|Parameters")
    void SetEffectInstanceScale(const FName& InstanceID, float Scale);

    // ========== 特效实例查询 ==========

    // 获取特效实例信息
    UFUNCTION(BlueprintCallable, Category = "Effect")
    bool GetEffectInstanceInfo(const FName& InstanceID, FEffectInstanceInfo& OutInfo) const;

    // 检查特效实例是否存在
    UFUNCTION(BlueprintCallable, Category = "Effect")
    bool DoesEffectInstanceExist(const FName& InstanceID) const;

    // 获取特效实例组件
    UFUNCTION(BlueprintCallable, Category = "Effect")
    UNiagaraComponent* GetEffectInstanceComponent(const FName& InstanceID) const;

    // 获取所有活跃的特效实例ID
    UFUNCTION(BlueprintCallable, Category = "Effect")
    TArray<FName> GetAllActiveEffectInstanceIDs() const;

    // 获取指定特效类型的所有实例ID
    UFUNCTION(BlueprintCallable, Category = "Effect")
    TArray<FName> GetAllEffectInstanceIDsOfType(const FName& EffectID) const;

    // ========== 批量操作 ==========

    // 批量停止所有特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect|Batch")
    void StopAllEffects(bool bImmediate = false);

    // 批量销毁所有特效实例
    UFUNCTION(BlueprintCallable, Category = "Effect|Batch")
    void DestroyAllEffects();

    // 批量销毁指定特效ID的所有实例
    UFUNCTION(BlueprintCallable, Category = "Effect|Batch")
    void DestroyAllEffectsOfType(const FName& EffectID);

    // 批量停止指定特效ID的所有实例
    UFUNCTION(BlueprintCallable, Category = "Effect|Batch")
    void StopAllEffectsOfType(const FName& EffectID, bool bImmediate = false);

    // ========== 简化回调接口 ==========

    // 播放特效并绑定完成回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Effect", meta = (DisplayName = "Play Effect With Finished Callback"))
    FName PlayEffectAtLocationWithFinishedCallback(const FName& EffectID, const FVector& Location, const FRotator& Rotation,
        const FOnEffectFinishedCallback& FinishedCallback);

    // 播放特效并绑定销毁回调（蓝图）
    UFUNCTION(BlueprintCallable, Category = "Effect", meta = (DisplayName = "Play Effect With Destroy Callback"))
    FName PlayEffectAtLocationWithDestroyCallback(const FName& EffectID, const FVector& Location, const FRotator& Rotation,
        const FOnEffectDestroyedCallback& DestroyedCallback);

private:
    // 流式加载管理器
    FStreamableManager StreamableManager;

    // 特效数据表
    UPROPERTY()
    UDataTable* EffectDataTable = nullptr;

    // 特效实例映射
    TMap<FName, FEffectInstanceInfo> EffectInstances;

    // 特效ID到配置的映射
    TMap<FName, FEffectTableRow> EffectConfigMap;

    // 回调映射
    TMap<FName, FOnEffectFinishedCallback> EffectFinishedCallbacks;
    TMap<FName, FOnEffectDestroyedCallback> EffectDestroyedCallbacks;

    // 延迟激活的定时器句柄
    TMap<FName, FTimerHandle> DelayActivationTimers;

    // 内部播放方法
    FName InternalPlayEffectAtLocation(const FName& EffectID, const FEffectTableRow& Config,
        const FVector& Location, const FRotator& Rotation);

    FName InternalPlayEffectAttached(const FName& EffectID, const FEffectTableRow& Config,
        AActor* ParentActor, FName SocketName,
        const FVector& RelativeOffset, const FRotator& RelativeRotation);

    FName InternalPlayEffectFollowActor(const FName& EffectID, const FEffectTableRow& Config,
        AActor* TargetActor);

    // 创建特效组件
    UNiagaraComponent* CreateWorldEffectComponent(const FEffectTableRow& Config, AActor*& OutTempActor);

    UNiagaraComponent* CreateEffectComponent(const FEffectTableRow& Config,
        AActor* ParentActor = nullptr,
        FName SocketName = NAME_None,
        const FVector& RelativeOffset = FVector::ZeroVector,
        const FRotator& RelativeRotation = FRotator::ZeroRotator,
        bool bFollowActor = false);

    // 更新特效实例（手动调用）
    void UpdateEffectInstances(float DeltaTime);

    // 处理特效完成
    void HandleEffectFinished(const FName& InstanceID);

    // 处理特效销毁
    void HandleEffectDestroyed(const FName& InstanceID);

    // 处理延迟播放
    void HandleDelayedActivation(FName InstanceID);

    // 检查实例是否应该销毁
    bool ShouldDestroyInstance(const FEffectInstanceInfo& InstanceInfo) const;

    // 工具方法
    FName GenerateInstanceID() const;
    void CleanupDestroyedEffects();

    // 初始化实例信息
    void InitializeInstanceInfo(FEffectInstanceInfo& InstanceInfo);

    // 处理播放时机
    void HandlePlayTiming(const FName& InstanceID, const FEffectTableRow& Config);

    // 更新控制
    FTimerHandle UpdateTimerHandle;
    float UpdateInterval = 0.1f;
    bool bIsUpdating = false;

    // 标记是否正在清理
    bool bIsCleaningUp = false;

    UWorld* GetWorld() const override;
};