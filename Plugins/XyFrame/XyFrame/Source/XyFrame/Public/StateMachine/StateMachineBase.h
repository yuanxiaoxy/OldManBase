// StateMachineBase.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SingletonBase/SingletonBase.h"
#include "MonoManager/MonoManager.h"
#include "StateMachineBase.generated.h"

// ״̬��ӵ���߽ӿ�
UINTERFACE(Blueprintable)
class UStateMachineOwner : public UInterface
{
    GENERATED_BODY()
};

class IStateMachineOwner
{
    GENERATED_BODY()
};

// ״̬����
UCLASS(Abstract, Blueprintable)
class XYFRAME_API UStateBase : public UObject
{
    GENERATED_BODY()

public:
    virtual ~UStateBase() = default;

    // ��ʼ���ڲ�����
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void InitializeInternal(class UStateMachineBase* InStateMachine)
    {
        StateMachine = InStateMachine;
    }

    // ��ʼ��״̬ - �򻯰汾��������ͼ����ǩ��
    virtual void Initialize(TScriptInterface<IStateMachineOwner> InOwner)
    {
        Owner = InOwner;
    }

    // ״̬����
    virtual void Enter() {}

    // ״̬�˳�  
    virtual void Exit() {}

    // ״̬����
    virtual void Update(float DeltaTime) {}

    // ״̬�ӳٸ���
    virtual void LateUpdate(float DeltaTime) {}

    // ״̬�̶�����
    virtual void FixedUpdate(float DeltaTime) {}

protected:
    // ����ת�����
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool CheckTransition(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent = false);

    UPROPERTY()
    class UStateMachineBase* StateMachine;

    UPROPERTY()
    TScriptInterface<IStateMachineOwner> Owner;
};

// ״̬������
UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UStateMachineBase : public UObject
{
    GENERATED_BODY()

public:
    UStateMachineBase();
    virtual ~UStateMachineBase();

    // ��ʼ��״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Initialize(TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing = false);

    // ����ʼ״̬�ĳ�ʼ��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void InitializeWithState(TSubclassOf<UStateBase> InitialState, TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing = false);

    // �ı�״̬
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    bool ChangeState(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent = false);

    // ��ȡ��ǰ״̬
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    UStateBase* GetCurrentState() const { return CurrentState; }

    // ��ȡ��ǰ״̬��
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    TSubclassOf<UStateBase> GetCurrentStateClass() const { return CurrentStateClass; }

    // ֹͣ״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Stop();

    // ����״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    virtual void Destroy();

    // ״̬�������ݲ���
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void SetSharedData(const FString& Key, UObject* Data);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    UObject* GetSharedData(const FString& Key) const;

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void RemoveSharedData(const FString& Key);

    // ����Ƿ���������
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "StateMachine")
    bool IsRunning() const { return bIsRunning; }

    // �ֶ�����״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void Update(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void LateUpdate(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void FixedUpdate(float DeltaTime);

    // ��дGetWorld
    virtual UWorld* GetWorld() const override;

protected:
    // �ڲ�״̬����
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

// ״̬��������
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

    // ����״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    UStateMachineBase* CreateStateMachine(TScriptInterface<IStateMachineOwner> Owner, bool bEnableStateSharing = false);

    // ����״̬��
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    void DestroyStateMachine(UStateMachineBase* StateMachine);

    // ��ȡ��Ծ״̬������
    UFUNCTION(BlueprintCallable, Category = "StateMachine")
    int32 GetActiveStateMachineCount() const { return ActiveStateMachines.Num(); }

private:
    UPROPERTY()
    TArray<UStateMachineBase*> ActiveStateMachines;
};