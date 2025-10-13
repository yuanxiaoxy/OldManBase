// StateMachineBase.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "MonoManager/MonoManager.h"
#include "StateMachineBase.generated.h"

// 状态机拥有者接口
UINTERFACE(Blueprintable)
class UStateMachineOwner : public UInterface
{
    GENERATED_BODY()
};

class IStateMachineOwner
{
    GENERATED_BODY()
};

// 状态基类
UCLASS(Abstract, Blueprintable)
class XYFRAME_API UStateBase : public UObject
{
    GENERATED_BODY()

public:
    virtual ~UStateBase() = default;

    // 初始化内部数据
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void InitializeInternal(class UStateMachineBase* InStateMachine)
    {
        StateMachine = InStateMachine;
    }

    // 初始化状态 - 简化版本，避免蓝图复杂签名
    virtual void Initialize(TScriptInterface<IStateMachineOwner> InOwner)
    {
        Owner = InOwner;
    }

    // 状态进入
    virtual void Enter() {}

    // 状态退出  
    virtual void Exit() {}

    // 状态更新
    virtual void Update(float DeltaTime) {}

    // 状态延迟更新
    virtual void LateUpdate(float DeltaTime) {}

    // 状态固定更新
    virtual void FixedUpdate(float DeltaTime) {}

protected:
    // 条件转换检查
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool CheckTransition(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent = false);

    UPROPERTY()
    class UStateMachineBase* StateMachine;

    UPROPERTY()
    TScriptInterface<IStateMachineOwner> Owner;
};

// 状态机基类
UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UStateMachineBase : public UObject
{
    GENERATED_BODY()

public:
    UStateMachineBase();
    virtual ~UStateMachineBase();

    // 初始化状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Initialize(TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing = false);

    // 带初始状态的初始化
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void InitializeWithState(TSubclassOf<UStateBase> InitialState, TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing = false);

    // 改变状态
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool ChangeState(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent = false);

    // 获取当前状态
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    UStateBase* GetCurrentState() const { return CurrentState; }

    // 获取当前状态类
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    TSubclassOf<UStateBase> GetCurrentStateClass() const { return CurrentStateClass; }

    // 停止状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Stop();

    // 销毁状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Destroy();

    // 状态共享数据操作
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void SetSharedData(const FString& Key, UObject* Data);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    UObject* GetSharedData(const FString& Key) const;

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void RemoveSharedData(const FString& Key);

    // 检查是否正在运行
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    bool IsRunning() const { return bIsRunning; }

    // 手动更新状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void Update(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void LateUpdate(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void FixedUpdate(float DeltaTime);

    // 重写GetWorld
    virtual UWorld* GetWorld() const override;

protected:
    // 内部状态创建
    UStateBase* GetOrCreateState(TSubclassOf<UStateBase> StateClass);

private:
    UPROPERTY()
    TScriptInterface<IStateMachineOwner> Owner;

    UPROPERTY()
    UStateBase* CurrentState;

    UPROPERTY()
    TSubclassOf<UStateBase> CurrentStateClass;

    UPROPERTY()
    TMap<TSubclassOf<UStateBase>, UStateBase*> StateMap;

    UPROPERTY()
    TMap<FString, UObject*> SharedDataMap;

    bool bIsRunning;
    bool bEnableStateSharing;
};

// 状态机管理器
UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UStateMachineManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UStateMachineManager)

public:
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void InitializeStateMachineManager();

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine", meta = (DisplayName = "Get StateMachine Manager"))
    static UStateMachineManager* GetStateMachineManager() { return GetInstance(); }

    // 创建状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    UStateMachineBase* CreateStateMachine(TScriptInterface<IStateMachineOwner> Owner, bool bEnableStateSharing = false);

    // 销毁状态机
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void DestroyStateMachine(UStateMachineBase* StateMachine);

    // 获取活跃状态机数量
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    int32 GetActiveStateMachineCount() const { return ActiveStateMachines.Num(); }

private:
    UPROPERTY()
    TArray<UStateMachineBase*> ActiveStateMachines;
};