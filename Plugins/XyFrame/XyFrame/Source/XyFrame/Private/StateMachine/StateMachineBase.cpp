// StateMachineBase.cpp
#include "StateMachine/StateMachineBase.h"
#include "Engine/Engine.h"

// ========== UStateBase ʵ�� ==========

bool UStateBase::CheckTransition(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent)
{
    if (StateMachine && NewStateClass)
    {
        return StateMachine->ChangeState(NewStateClass, bReEnterCurrent);
    }
    return false;
}

// ========== UStateMachineBase ʵ�� ==========

UStateMachineBase::UStateMachineBase()
    : CurrentState(nullptr)
    , CurrentStateClass(nullptr)
    , bIsRunning(false)
    , bEnableStateSharing(false)
{
}

UStateMachineBase::~UStateMachineBase()
{
    Stop();
}

void UStateMachineBase::Initialize(TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing)
{
    Owner = InOwner;
    bEnableStateSharing = bInEnableStateSharing;
    bIsRunning = false;

    // �����������
    CurrentState = nullptr;
    CurrentStateClass = nullptr;
    StateMap.Empty();

    if (bEnableStateSharing)
    {
        SharedDataMap.Empty();
    }

    UE_LOG(LogTemp, Log, TEXT("StateMachine initialized for owner: %s"),
        Owner.GetObject() ? *Owner.GetObject()->GetName() : TEXT("None"));
}

void UStateMachineBase::InitializeWithState(TSubclassOf<UStateBase> InitialState, TScriptInterface<IStateMachineOwner> InOwner, bool bInEnableStateSharing)
{
    Initialize(InOwner, bInEnableStateSharing);
    ChangeState(InitialState);
}

bool UStateMachineBase::ChangeState(TSubclassOf<UStateBase> NewStateClass, bool bReEnterCurrent)
{
    if (!NewStateClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot change to null state class"));
        return false;
    }

    // ״̬��ͬ�Ҳ��������½��뵱ǰ״̬
    if (NewStateClass == CurrentStateClass && !bReEnterCurrent)
    {
        return false;
    }

    // �˳���ǰ״̬
    if (CurrentState)
    {
        CurrentState->Exit();
        UE_LOG(LogTemp, Log, TEXT("Exited state: %s"), *CurrentStateClass->GetName());
    }

    // ������״̬
    CurrentState = GetOrCreateState(NewStateClass);
    CurrentStateClass = NewStateClass;

    if (CurrentState)
    {
        CurrentState->Enter();
        UE_LOG(LogTemp, Log, TEXT("Entered state: %s"), *CurrentStateClass->GetName());
        bIsRunning = true;
    }

    return true;
}

UStateBase* UStateMachineBase::GetOrCreateState(TSubclassOf<UStateBase> StateClass)
{
    if (!StateClass)
    {
        return nullptr;
    }

    // ��ӳ���в�������״̬
    UStateBase** ExistingState = StateMap.Find(StateClass);
    if (ExistingState && *ExistingState)
    {
        return *ExistingState;
    }

    // ������״̬
    UStateBase* NewState = NewObject<UStateBase>(this, StateClass);
    if (NewState)
    {
        NewState->InitializeInternal(this);
        NewState->Initialize(Owner);
        StateMap.Add(StateClass, NewState);

        UE_LOG(LogTemp, Log, TEXT("Created new state: %s"), *StateClass->GetName());
    }

    return NewState;
}

void UStateMachineBase::Stop()
{
    if (IsValid(CurrentState) && CurrentState->IsNative())
    {
        CurrentState->Exit();
        CurrentState = nullptr;
        CurrentStateClass = nullptr;
    }

    bIsRunning = false;
    UE_LOG(LogTemp, Log, TEXT("StateMachine stopped"));
}

void UStateMachineBase::Destroy()
{
    Stop();

    // ��������״̬
    StateMap.Empty();
    SharedDataMap.Empty();

    UE_LOG(LogTemp, Log, TEXT("StateMachine destroyed"));
}

void UStateMachineBase::SetSharedData(const FString& Key, UObject* Data)
{
    if (bEnableStateSharing)
    {
        SharedDataMap.Add(Key, Data);
    }
}

UObject* UStateMachineBase::GetSharedData(const FString& Key) const
{
    if (bEnableStateSharing)
    {
        UObject* const* DataPtr = SharedDataMap.Find(Key);
        return DataPtr ? *DataPtr : nullptr;
    }
    return nullptr;
}

void UStateMachineBase::RemoveSharedData(const FString& Key)
{
    if (bEnableStateSharing)
    {
        SharedDataMap.Remove(Key);
    }
}

void UStateMachineBase::Update(float DeltaTime)
{
    if (CurrentState && bIsRunning)
    {
        CurrentState->Update(DeltaTime);
    }
}

void UStateMachineBase::LateUpdate(float DeltaTime)
{
    if (CurrentState && bIsRunning)
    {
        CurrentState->LateUpdate(DeltaTime);
    }
}

void UStateMachineBase::FixedUpdate(float DeltaTime)
{
    if (CurrentState && bIsRunning)
    {
        CurrentState->FixedUpdate(DeltaTime);
    }
}

UWorld* UStateMachineBase::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                return Context.World();
            }
        }
    }
    return nullptr;
}

// ========== UStateMachineManager ʵ�� ==========

// ��̬ʵ������
template<>
UStateMachineManager* TSingleton<UStateMachineManager>::SingletonInstance = nullptr;

void UStateMachineManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("StateMachineManager InitializeSingleton called"));
    InitializeStateMachineManager();
}

void UStateMachineManager::InitializeStateMachineManager()
{
    UE_LOG(LogTemp, Log, TEXT("StateMachine Manager Initialized"));
}

UStateMachineBase* UStateMachineManager::CreateStateMachine(TScriptInterface<IStateMachineOwner> Owner, bool bEnableStateSharing)
{
    UStateMachineBase* NewStateMachine = NewObject<UStateMachineBase>();
    if (NewStateMachine)
    {
        NewStateMachine->Initialize(Owner, bEnableStateSharing);
        ActiveStateMachines.Add(NewStateMachine);

        UE_LOG(LogTemp, Log, TEXT("Created new state machine"));
    }

    return NewStateMachine;
}

void UStateMachineManager::DestroyStateMachine(UStateMachineBase* StateMachine)
{
    if (StateMachine && ActiveStateMachines.Contains(StateMachine))
    {
        StateMachine->Destroy();
        ActiveStateMachines.Remove(StateMachine);

        UE_LOG(LogTemp, Log, TEXT("Destroyed state machine"));
    }
}