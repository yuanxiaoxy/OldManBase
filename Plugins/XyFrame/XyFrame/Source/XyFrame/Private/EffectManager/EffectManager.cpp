// Fill out your copyright notice in the Description page of Project Settings.

#include "EffectManager/EffectManager.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

// 静态实例定义
template<>
UEffectManager* TSingleton<UEffectManager>::SingletonInstance = nullptr;

// 辅助函数：查找适合附加的组件（优先查找包含Socket的StaticMesh/SkeletalMesh组件）
static USceneComponent* FindComponentForAttach(AActor* Actor, FName SocketName)
{
    if (!Actor) return nullptr;

    if (SocketName != NAME_None)
    {
        TArray<UStaticMeshComponent*> StaticComps;
        Actor->GetComponents<UStaticMeshComponent>(StaticComps);
        for (auto* Comp : StaticComps)
        {
            if (Comp && Comp->DoesSocketExist(SocketName))
                return Comp;
        }

        TArray<USkeletalMeshComponent*> SkelComps;
        Actor->GetComponents<USkeletalMeshComponent>(SkelComps);
        for (auto* Comp : SkelComps)
        {
            if (Comp && Comp->DoesSocketExist(SocketName))
                return Comp;
        }

        TArray<USceneComponent*> SceneComps;
        Actor->GetComponents<USceneComponent>(SceneComps);
        for (auto* Comp : SceneComps)
        {
            if (Comp && Comp->DoesSocketExist(SocketName))
                return Comp;
        }

        UE_LOG(LogTemp, Warning, TEXT("Socket %s not found on actor %s, falling back to root component"),
            *SocketName.ToString(), *Actor->GetName());
    }

    return Actor->GetRootComponent();
}

UEffectManager::UEffectManager()
{
    bIsUpdating = false;
    bIsCleaningUp = false;
    UpdateInterval = 0.1f;
}

UEffectManager::~UEffectManager()
{
    StopUpdating();

    for (auto& TimerPair : DelayActivationTimers)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(TimerPair.Value);
        }
    }
    DelayActivationTimers.Empty();

    bIsCleaningUp = true;
    DestroyAllEffects();
    bIsCleaningUp = false;

    EffectInstances.Empty();
    EffectConfigMap.Empty();
    EffectFinishedCallbacks.Empty();
    EffectDestroyedCallbacks.Empty();
}

void UEffectManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("EffectManager InitializeSingleton called"));
    InitializeEffectManager();
}

void UEffectManager::DestroyCurSingleton()
{
    StopUpdating();
    bIsCleaningUp = true;
    DestroyAllEffects();
    bIsCleaningUp = false;
    DestroyInstance();
}

void UEffectManager::InitializeEffectManager()
{
    UE_LOG(LogTemp, Log, TEXT("Effect Manager Initialized"));
}

// ========== 更新控制 ==========

void UEffectManager::StartUpdating(float Interval)
{
    if (bIsUpdating) return;
    if (Interval <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Update interval must be greater than 0"));
        return;
    }

    UpdateInterval = Interval;
    UWorld* World = GetWorld();
    if (World && World->IsGameWorld())
    {
        World->GetTimerManager().SetTimer(UpdateTimerHandle, [this]() {
            if (this) UpdateEffectManager(UpdateInterval);
            }, UpdateInterval, true);
        bIsUpdating = true;
        UE_LOG(LogTemp, Log, TEXT("Effect Manager started updating with interval: %.2f"), UpdateInterval);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start effect manager update: World is null or not a game world"));
    }
}

void UEffectManager::StopUpdating()
{
    if (!bIsUpdating) return;
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
        bIsUpdating = false;
        UE_LOG(LogTemp, Log, TEXT("Effect Manager stopped updating"));
    }
}

void UEffectManager::UpdateEffectManager(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        StopUpdating();
        CleanupDestroyedEffects();
        return;
    }
    UpdateEffectInstances(DeltaTime);
}

// ========== 数据表相关操作 ==========

void UEffectManager::SetEffectDataTable(UDataTable* InEffectDataTable)
{
    EffectDataTable = InEffectDataTable;
    EffectConfigMap.Empty();

    if (EffectDataTable)
    {
        TArray<FEffectTableRow*> AllRows;
        EffectDataTable->GetAllRows<FEffectTableRow>(TEXT("EffectManager"), AllRows);
        for (const FEffectTableRow* Row : AllRows)
        {
            if (Row && !Row->EffectID.IsNone())
            {
                EffectConfigMap.Add(Row->EffectID, *Row);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("Effect DataTable set, loaded %d effects"), EffectConfigMap.Num());
    }
}

bool UEffectManager::GetEffectConfig(const FName& EffectID, FEffectTableRow& OutConfig) const
{
    const FEffectTableRow* Config = EffectConfigMap.Find(EffectID);
    if (Config)
    {
        OutConfig = *Config;
        return true;
    }
    return false;
}

bool UEffectManager::DoesEffectIDExist(const FName& EffectID) const
{
    return EffectConfigMap.Contains(EffectID);
}

// ========== 特效播放控制 ==========

FName UEffectManager::PlayEffectAtLocation(const FName& EffectID, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play effect: World is null or not a game world"));
        return NAME_None;
    }

    FEffectTableRow Config;
    if (!GetEffectConfig(EffectID, Config))
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect ID %s not found in config"), *EffectID.ToString());
        return NAME_None;
    }
    return InternalPlayEffectAtLocation(EffectID, Config, Location, Rotation);
}

FName UEffectManager::PlayEffectAttached(const FName& EffectID, AActor* TargetActor,
    FName SocketName,
    const FVector& RelativeOffset,
    const FRotator& RelativeRotation)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play effect: World is null or not a game world"));
        return NAME_None;
    }
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("TargetActor is null for effect %s"), *EffectID.ToString());
        return NAME_None;
    }

    FEffectTableRow Config;
    if (!GetEffectConfig(EffectID, Config))
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect ID %s not found in config"), *EffectID.ToString());
        return NAME_None;
    }

    return InternalPlayEffectAttached(EffectID, Config, TargetActor, SocketName,
        RelativeOffset.IsZero() ? Config.RelativeOffset : RelativeOffset,
        RelativeRotation.IsZero() ? Config.RelativeRotation : RelativeRotation);
}

FName UEffectManager::PlayEffectFollowActor(const FName& EffectID, AActor* TargetActor)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play effect: World is null or not a game world"));
        return NAME_None;
    }
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("TargetActor is null for effect %s"), *EffectID.ToString());
        return NAME_None;
    }

    FEffectTableRow Config;
    if (!GetEffectConfig(EffectID, Config))
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect ID %s not found in config"), *EffectID.ToString());
        return NAME_None;
    }
    return InternalPlayEffectFollowActor(EffectID, Config, TargetActor);
}

// ========== 内部播放方法实现 ==========

FName UEffectManager::InternalPlayEffectAtLocation(const FName& EffectID, const FEffectTableRow& Config,
    const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return NAME_None;

    FName InstanceID = GenerateInstanceID();
    AActor* TempActor = nullptr;
    UNiagaraComponent* EffectComponent = CreateWorldEffectComponent(Config, TempActor);
    if (!EffectComponent) return NAME_None;

    EffectComponent->SetWorldLocationAndRotation(Location, Rotation);
    if (Config.SizeMultiplier != 1.0f)
        EffectComponent->SetWorldScale3D(FVector(Config.SizeMultiplier));

    FEffectInstanceInfo InstanceInfo;
    InstanceInfo.InstanceID = InstanceID;
    InstanceInfo.EffectID = EffectID;
    InstanceInfo.EffectComponent = EffectComponent;
    InstanceInfo.TempActor = TempActor;
    InstanceInfo.StartTime = World->GetTimeSeconds();
    InstanceInfo.EffectConfig = Config;
    InstanceInfo.bIsFollowMode = false;
    InitializeInstanceInfo(InstanceInfo);

    EffectInstances.Add(InstanceID, InstanceInfo);
    HandlePlayTiming(InstanceID, Config);

    UE_LOG(LogTemp, Log, TEXT("Created effect instance at location: %s (Effect: %s)"), *InstanceID.ToString(), *EffectID.ToString());
    return InstanceID;
}

FName UEffectManager::InternalPlayEffectAttached(const FName& EffectID, const FEffectTableRow& Config,
    AActor* ParentActor, FName SocketName,
    const FVector& RelativeOffset, const FRotator& RelativeRotation)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return NAME_None;
    if (!ParentActor || !IsValid(ParentActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ParentActor is invalid for effect %s"), *EffectID.ToString());
        return NAME_None;
    }

    FName InstanceID = GenerateInstanceID();
    UNiagaraComponent* EffectComponent = CreateEffectComponent(Config, ParentActor, SocketName,
        RelativeOffset, RelativeRotation, false);
    if (!EffectComponent) return NAME_None;

    FEffectInstanceInfo InstanceInfo;
    InstanceInfo.InstanceID = InstanceID;
    InstanceInfo.EffectID = EffectID;
    InstanceInfo.EffectComponent = EffectComponent;
    InstanceInfo.ParentActor = ParentActor;
    InstanceInfo.StartTime = World->GetTimeSeconds();
    InstanceInfo.EffectConfig = Config;
    InstanceInfo.bIsFollowMode = false;
    InitializeInstanceInfo(InstanceInfo);

    EffectInstances.Add(InstanceID, InstanceInfo);
    HandlePlayTiming(InstanceID, Config);

    UE_LOG(LogTemp, Log, TEXT("Created effect instance attached: %s (Effect: %s)"), *InstanceID.ToString(), *EffectID.ToString());
    return InstanceID;
}

FName UEffectManager::InternalPlayEffectFollowActor(const FName& EffectID, const FEffectTableRow& Config,
    AActor* TargetActor)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return NAME_None;
    if (!TargetActor || !IsValid(TargetActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("TargetActor is invalid for effect %s"), *EffectID.ToString());
        return NAME_None;
    }

    FName InstanceID = GenerateInstanceID();
    UNiagaraComponent* EffectComponent = CreateEffectComponent(Config, TargetActor,
        NAME_None, Config.RelativeOffset, Config.RelativeRotation, true);
    if (!EffectComponent) return NAME_None;

    FEffectInstanceInfo InstanceInfo;
    InstanceInfo.InstanceID = InstanceID;
    InstanceInfo.EffectID = EffectID;
    InstanceInfo.EffectComponent = EffectComponent;
    InstanceInfo.ParentActor = TargetActor;
    InstanceInfo.StartTime = World->GetTimeSeconds();
    InstanceInfo.EffectConfig = Config;
    InstanceInfo.bIsFollowMode = true;
    InitializeInstanceInfo(InstanceInfo);

    EffectInstances.Add(InstanceID, InstanceInfo);
    HandlePlayTiming(InstanceID, Config);

    UE_LOG(LogTemp, Log, TEXT("Created effect instance follow actor: %s (Effect: %s)"), *InstanceID.ToString(), *EffectID.ToString());
    return InstanceID;
}

// ========== 特效实例控制 ==========

EEffectInstanceState UEffectManager::GetEffectInstanceState(const FName& InstanceID) const
{
    const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (!InstanceInfo || !InstanceInfo->EffectComponent || !IsValid(InstanceInfo->EffectComponent))
        return EEffectInstanceState::Destroyed;

    if (!InstanceInfo->bIsActive) return EEffectInstanceState::Created;
    if (!InstanceInfo->bIsPlaying) return (InstanceInfo->EffectConfig.PlayTiming == EEffectPlayTiming::Delayed) ? EEffectInstanceState::Delayed : EEffectInstanceState::Created;
    if (InstanceInfo->EffectComponent && !InstanceInfo->EffectComponent->IsActive()) return EEffectInstanceState::Finished;
    return EEffectInstanceState::Active;
}

void UEffectManager::ActivateEffectInstance(const FName& InstanceID)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent) && !InstanceInfo->bIsActive)
    {
        UWorld* World = GetWorld();
        if (!World || !World->IsGameWorld()) return;
        InstanceInfo->EffectComponent->Activate(true);
        InstanceInfo->bIsActive = true;
        InstanceInfo->bIsPlaying = true;
        InstanceInfo->StartTime = World->GetTimeSeconds();
        UE_LOG(LogTemp, Log, TEXT("Activated effect instance: %s"), *InstanceID.ToString());
    }
}

void UEffectManager::StopEffectInstance(const FName& InstanceID, bool bImmediate)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
    {
        if (bImmediate) InstanceInfo->EffectComponent->DeactivateImmediate();
        else InstanceInfo->EffectComponent->Deactivate();
        InstanceInfo->bIsPlaying = false;
        InstanceInfo->bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Stopped effect instance: %s"), *InstanceID.ToString());
    }
}

void UEffectManager::DestroyEffectInstance(const FName& InstanceID)
{
    if (bIsCleaningUp)
    {
        EffectInstances.Remove(InstanceID);
        DelayActivationTimers.Remove(InstanceID);
        EffectFinishedCallbacks.Remove(InstanceID);
        EffectDestroyedCallbacks.Remove(InstanceID);
        return;
    }

    FEffectInstanceInfo InstanceInfo;
    if (EffectInstances.RemoveAndCopyValue(InstanceID, InstanceInfo))
    {
        if (FTimerHandle* TimerHandle = DelayActivationTimers.Find(InstanceID))
        {
            if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(*TimerHandle);
            DelayActivationTimers.Remove(InstanceID);
        }

        if (InstanceInfo.EffectComponent && IsValid(InstanceInfo.EffectComponent))
        {
            InstanceInfo.EffectComponent->DeactivateImmediate();
            if (!InstanceInfo.EffectComponent->IsBeingDestroyed())
            {
                InstanceInfo.EffectComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
                InstanceInfo.EffectComponent->DestroyComponent();
            }
        }

        if (InstanceInfo.TempActor && IsValid(InstanceInfo.TempActor))
            InstanceInfo.TempActor->Destroy();

        HandleEffectDestroyed(InstanceID);
        EffectFinishedCallbacks.Remove(InstanceID);
        EffectDestroyedCallbacks.Remove(InstanceID);
        UE_LOG(LogTemp, Log, TEXT("Destroyed effect instance: %s"), *InstanceID.ToString());
    }
}

void UEffectManager::RestartEffectInstance(const FName& InstanceID)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
    {
        UWorld* World = GetWorld();
        if (!World || !World->IsGameWorld()) return;
        StopEffectInstance(InstanceID, true);
        InstanceInfo->StartTime = World->GetTimeSeconds();
        InstanceInfo->CurrentTime = 0.0f;
        InstanceInfo->RemainingLifetime = InstanceInfo->EffectConfig.Lifetime;
        InstanceInfo->RemainingLoops = InstanceInfo->EffectConfig.LoopCount;
        ActivateEffectInstance(InstanceID);
        UE_LOG(LogTemp, Log, TEXT("Restarted effect instance: %s"), *InstanceID.ToString());
    }
}

bool UEffectManager::IsEffectInstanceValid(const FName& InstanceID) const
{
    const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    return InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent);
}

// ========== 新增：位置/变换控制 ==========

void UEffectManager::SetEffectWorldLocation(const FName& InstanceID, const FVector& NewLocation)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
    {
        InstanceInfo->EffectComponent->SetWorldLocation(NewLocation);
    }
}

void UEffectManager::SetEffectWorldRotation(const FName& InstanceID, const FRotator& NewRotation)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
    {
        InstanceInfo->EffectComponent->SetWorldRotation(NewRotation);
    }
}

void UEffectManager::SetEffectWorldTransform(const FName& InstanceID, const FTransform& NewTransform)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo && InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
    {
        InstanceInfo->EffectComponent->SetWorldTransform(NewTransform);
    }
}

void UEffectManager::SetEffectFollowLocationTarget(const FName& InstanceID, AActor* TargetActor, FVector Offset)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo)
    {
        if (TargetActor && IsValid(TargetActor))
        {
            InstanceInfo->FollowLocationTarget = TargetActor;
            InstanceInfo->FollowLocationOffset = Offset;
            // 立即更新一次位置
            FVector NewLocation = TargetActor->GetActorLocation() + Offset;
            SetEffectWorldLocation(InstanceID, NewLocation);
        }
        else
        {
            ClearEffectFollowLocationTarget(InstanceID);
        }
    }
}

void UEffectManager::ClearEffectFollowLocationTarget(const FName& InstanceID)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo)
    {
        InstanceInfo->FollowLocationTarget.Reset();
        InstanceInfo->FollowLocationOffset = FVector::ZeroVector;
    }
}

// ========== 特效实例参数控制 ==========

void UEffectManager::SetEffectFloatParameter(const FName& InstanceID, const FName& ParameterName, float Value)
{
    if (FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
        if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
            InstanceInfo->EffectComponent->SetFloatParameter(ParameterName, Value);
}

void UEffectManager::SetEffectVectorParameter(const FName& InstanceID, const FName& ParameterName, const FVector& Value)
{
    if (FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
        if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
            InstanceInfo->EffectComponent->SetVectorParameter(ParameterName, Value);
}

void UEffectManager::SetEffectColorParameter(const FName& InstanceID, const FName& ParameterName, const FLinearColor& Value)
{
    if (FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
        if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
            InstanceInfo->EffectComponent->SetColorParameter(ParameterName, Value);
}

void UEffectManager::SetEffectInstanceColor(const FName& InstanceID, const FLinearColor& Color)
{
    if (FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
        if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
        {
            InstanceInfo->EffectComponent->SetColorParameter(FName("Color"), Color);
            InstanceInfo->EffectComponent->SetColorParameter(FName("BaseColor"), Color);
        }
}

void UEffectManager::SetEffectInstanceScale(const FName& InstanceID, float Scale)
{
    if (FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
        if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
        {
            InstanceInfo->EffectComponent->SetWorldScale3D(FVector(Scale));
            InstanceInfo->EffectConfig.SizeMultiplier = Scale;
        }
}

// ========== 特效实例查询 ==========

bool UEffectManager::GetEffectInstanceInfo(const FName& InstanceID, FEffectInstanceInfo& OutInfo) const
{
    if (const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID))
    {
        OutInfo = *InstanceInfo;
        return true;
    }
    return false;
}

bool UEffectManager::DoesEffectInstanceExist(const FName& InstanceID) const
{
    return EffectInstances.Contains(InstanceID);
}

UNiagaraComponent* UEffectManager::GetEffectInstanceComponent(const FName& InstanceID) const
{
    const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    return (InstanceInfo && IsValid(InstanceInfo->EffectComponent)) ? InstanceInfo->EffectComponent : nullptr;
}

TArray<FName> UEffectManager::GetAllActiveEffectInstanceIDs() const
{
    TArray<FName> ActiveInstanceIDs;
    for (const auto& Pair : EffectInstances)
        if (Pair.Value.bIsPlaying && IsValid(Pair.Value.EffectComponent))
            ActiveInstanceIDs.Add(Pair.Key);
    return ActiveInstanceIDs;
}

TArray<FName> UEffectManager::GetAllEffectInstanceIDsOfType(const FName& EffectID) const
{
    TArray<FName> InstanceIDs;
    for (const auto& Pair : EffectInstances)
        if (Pair.Value.EffectID == EffectID && IsValid(Pair.Value.EffectComponent))
            InstanceIDs.Add(Pair.Key);
    return InstanceIDs;
}

// ========== 批量操作 ==========

void UEffectManager::StopAllEffects(bool bImmediate)
{
    TArray<FName> InstanceIDs;
    EffectInstances.GetKeys(InstanceIDs);
    for (const FName& ID : InstanceIDs)
        StopEffectInstance(ID, bImmediate);
}

void UEffectManager::DestroyAllEffects()
{
    if (bIsCleaningUp) return;
    bIsCleaningUp = true;

    for (auto& TimerPair : DelayActivationTimers)
        if (UWorld* World = GetWorld())
            World->GetTimerManager().ClearTimer(TimerPair.Value);
    DelayActivationTimers.Empty();

    TArray<FName> InstanceIDs;
    EffectInstances.GetKeys(InstanceIDs);
    for (const FName& ID : InstanceIDs)
    {
        FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(ID);
        if (InstanceInfo)
        {
            if (InstanceInfo->EffectComponent && IsValid(InstanceInfo->EffectComponent))
            {
                InstanceInfo->EffectComponent->DeactivateImmediate();
                if (!InstanceInfo->EffectComponent->IsBeingDestroyed())
                {
                    InstanceInfo->EffectComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
                    InstanceInfo->EffectComponent->DestroyComponent();
                }
            }
            if (InstanceInfo->TempActor && IsValid(InstanceInfo->TempActor))
                InstanceInfo->TempActor->Destroy();
            HandleEffectDestroyed(ID);
        }
    }

    EffectInstances.Empty();
    EffectFinishedCallbacks.Empty();
    EffectDestroyedCallbacks.Empty();
    bIsCleaningUp = false;
    UE_LOG(LogTemp, Log, TEXT("Destroyed all effects"));
}

void UEffectManager::DestroyAllEffectsOfType(const FName& EffectID)
{
    TArray<FName> InstancesToDestroy;
    for (const auto& Pair : EffectInstances)
        if (Pair.Value.EffectID == EffectID)
            InstancesToDestroy.Add(Pair.Key);
    for (const FName& ID : InstancesToDestroy)
        DestroyEffectInstance(ID);
}

void UEffectManager::StopAllEffectsOfType(const FName& EffectID, bool bImmediate)
{
    TArray<FName> InstancesToStop;
    for (const auto& Pair : EffectInstances)
        if (Pair.Value.EffectID == EffectID)
            InstancesToStop.Add(Pair.Key);
    for (const FName& ID : InstancesToStop)
        StopEffectInstance(ID, bImmediate);
    UE_LOG(LogTemp, Log, TEXT("Stopped %d effects of type %s"), InstancesToStop.Num(), *EffectID.ToString());
}

// ========== 简化回调接口 ==========

FName UEffectManager::PlayEffectAtLocationWithFinishedCallback(const FName& EffectID, const FVector& Location, const FRotator& Rotation,
    const FOnEffectFinishedCallback& FinishedCallback)
{
    FName InstanceID = PlayEffectAtLocation(EffectID, Location, Rotation);
    if (!InstanceID.IsNone() && FinishedCallback.IsBound())
        EffectFinishedCallbacks.Add(InstanceID, FinishedCallback);
    return InstanceID;
}

FName UEffectManager::PlayEffectAtLocationWithDestroyCallback(const FName& EffectID, const FVector& Location, const FRotator& Rotation,
    const FOnEffectDestroyedCallback& DestroyedCallback)
{
    FName InstanceID = PlayEffectAtLocation(EffectID, Location, Rotation);
    if (!InstanceID.IsNone() && DestroyedCallback.IsBound())
        EffectDestroyedCallbacks.Add(InstanceID, DestroyedCallback);
    return InstanceID;
}

// ========== 内部工具方法 ==========

UNiagaraComponent* UEffectManager::CreateWorldEffectComponent(const FEffectTableRow& Config, AActor*& OutTempActor)
{
    OutTempActor = nullptr;
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create world effect component: World is null or not a game world"));
        return nullptr;
    }

    UObject* EffectAsset = StreamableManager.LoadSynchronous(Config.EffectAssetPath);
    if (!EffectAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load effect asset: %s"), *Config.EffectAssetPath.ToString());
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(EffectAsset);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect asset is not a Niagara System: %s"), *EffectAsset->GetClass()->GetName());
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.ObjectFlags = RF_Transient;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    SpawnParams.bNoFail = true;
    AActor* TempActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (!TempActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to create temporary actor for world effect"));
        return nullptr;
    }
    TempActor->SetFlags(RF_Transient);

    UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(TempActor);
    if (!NiagaraComp)
    {
        TempActor->Destroy();
        return nullptr;
    }

    NiagaraComp->SetAsset(NiagaraSystem);
    NiagaraComp->RegisterComponent();
    NiagaraComp->SetUsingAbsoluteLocation(true);
    NiagaraComp->SetUsingAbsoluteRotation(true);
    NiagaraComp->SetUsingAbsoluteScale(true);
    NiagaraComp->SetActive(false, false);
    OutTempActor = TempActor;
    return NiagaraComp;
}

UNiagaraComponent* UEffectManager::CreateEffectComponent(const FEffectTableRow& Config,
    AActor* ParentActor,
    FName SocketName,
    const FVector& RelativeOffset,
    const FRotator& RelativeRotation,
    bool bFollowActor)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create effect component: World is null or not a game world"));
        return nullptr;
    }
    if (!ParentActor || !IsValid(ParentActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("ParentActor is invalid for effect"));
        return nullptr;
    }

    UObject* EffectAsset = StreamableManager.LoadSynchronous(Config.EffectAssetPath);
    if (!EffectAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load effect asset: %s"), *Config.EffectAssetPath.ToString());
        return nullptr;
    }
    UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(EffectAsset);
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Effect asset is not a Niagara System: %s"), *EffectAsset->GetClass()->GetName());
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = NewObject<UNiagaraComponent>(ParentActor);
    if (!NiagaraComp) return nullptr;

    NiagaraComp->SetAsset(NiagaraSystem);
    NiagaraComp->RegisterComponent();
    NiagaraComp->SetActive(false);

    if (!bFollowActor)
    {
        USceneComponent* TargetComponent = FindComponentForAttach(ParentActor, SocketName);
        if (!TargetComponent)
        {
            UE_LOG(LogTemp, Error, TEXT("No target component for attachment on actor %s"), *ParentActor->GetName());
        }
        else
        {
            FAttachmentTransformRules AttachRules = FAttachmentTransformRules::KeepRelativeTransform;
            bool bAttached = NiagaraComp->AttachToComponent(TargetComponent, AttachRules, SocketName);
            if (!bAttached)
            {
                UE_LOG(LogTemp, Warning, TEXT("AttachToComponent failed for %s to %s socket %s, trying SnapToTarget"),
                    *ParentActor->GetName(), *TargetComponent->GetName(), *SocketName.ToString());
                AttachRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
                bAttached = NiagaraComp->AttachToComponent(TargetComponent, AttachRules, SocketName);
                if (!bAttached)
                    UE_LOG(LogTemp, Error, TEXT("AttachToComponent still failed, effect may not follow parent"));
            }
        }

        NiagaraComp->SetRelativeLocation(RelativeOffset);
        NiagaraComp->SetRelativeRotation(RelativeRotation);

        // 缩放处理
        if (Config.bFollowScale)
        {
            NiagaraComp->SetRelativeScale3D(FVector(Config.SizeMultiplier));
            NiagaraComp->SetUsingAbsoluteScale(false);
        }
        else
        {
            NiagaraComp->SetUsingAbsoluteScale(true);
            NiagaraComp->SetWorldScale3D(FVector(Config.SizeMultiplier));
        }
    }
    else
    {
        NiagaraComp->SetUsingAbsoluteLocation(true);
        NiagaraComp->SetUsingAbsoluteRotation(true);
        NiagaraComp->SetUsingAbsoluteScale(true);

        FVector InitialLocation = ParentActor->GetActorLocation();
        if (!RelativeOffset.IsZero())
            InitialLocation += ParentActor->GetActorRotation().RotateVector(RelativeOffset);
        NiagaraComp->SetWorldLocation(InitialLocation);
        NiagaraComp->SetWorldRotation(ParentActor->GetActorRotation() + RelativeRotation);

        if (Config.bFollowScale)
        {
            FVector ParentScale = ParentActor->GetActorScale3D();
            NiagaraComp->SetWorldScale3D(ParentScale * Config.SizeMultiplier);
        }
        else
        {
            NiagaraComp->SetWorldScale3D(FVector(Config.SizeMultiplier));
        }
    }

    return NiagaraComp;
}

void UEffectManager::UpdateEffectInstances(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld() || bIsCleaningUp)
    {
        StopUpdating();
        CleanupDestroyedEffects();
        return;
    }

    TArray<FName> InstancesToDestroy;
    for (auto& Pair : EffectInstances)
    {
        FEffectInstanceInfo& Info = Pair.Value;

        if (!Info.EffectComponent || !IsValid(Info.EffectComponent) ||
            (Info.TempActor && !IsValid(Info.TempActor)) ||
            (Info.ParentActor && !IsValid(Info.ParentActor)))
        {
            InstancesToDestroy.Add(Pair.Key);
            continue;
        }

        if (Info.bIsActive && Info.bIsPlaying)
        {
            Info.CurrentTime += DeltaTime;
            switch (Info.EffectConfig.DestroyCondition)
            {
            case EEffectDestroyCondition::AfterLifetime:
                Info.RemainingLifetime -= DeltaTime;
                if (Info.RemainingLifetime <= 0.0f) InstancesToDestroy.Add(Pair.Key);
                break;
            case EEffectDestroyCondition::AfterLoops:
                if (Info.EffectComponent && !Info.EffectComponent->IsActive())
                {
                    if (Info.RemainingLoops > 0)
                    {
                        Info.RemainingLoops--;
                        if (Info.RemainingLoops <= 0) InstancesToDestroy.Add(Pair.Key);
                        else RestartEffectInstance(Pair.Key);
                    }
                }
                break;
            case EEffectDestroyCondition::OnFinish:
                if (Info.EffectComponent && !Info.EffectComponent->IsActive())
                    InstancesToDestroy.Add(Pair.Key);
                break;
            default: break;
            }

            // 手动跟随模式（原有逻辑）
            if (Info.bIsFollowMode && Info.ParentActor && IsValid(Info.ParentActor))
            {
                FVector TargetLocation = Info.ParentActor->GetActorLocation();
                if (!Info.EffectConfig.RelativeOffset.IsZero())
                    TargetLocation += Info.ParentActor->GetActorRotation().RotateVector(Info.EffectConfig.RelativeOffset);
                Info.EffectComponent->SetWorldLocation(TargetLocation);

                if (Info.EffectConfig.bFollowRotation)
                {
                    FRotator TargetRotation = Info.ParentActor->GetActorRotation();
                    TargetRotation += Info.EffectConfig.RelativeRotation;
                    Info.EffectComponent->SetWorldRotation(TargetRotation);
                }

                if (Info.EffectConfig.bFollowScale)
                {
                    FVector ParentScale = Info.ParentActor->GetActorScale3D();
                    Info.EffectComponent->SetWorldScale3D(ParentScale * Info.EffectConfig.SizeMultiplier);
                }
            }

            // ========== 新增：自动跟随指定Actor的位置 ==========
            if (Info.FollowLocationTarget.IsValid())
            {
                AActor* TargetActor = Info.FollowLocationTarget.Get();
                if (TargetActor && IsValid(TargetActor))
                {
                    FVector NewLocation = TargetActor->GetActorLocation() + Info.FollowLocationOffset;
                    Info.EffectComponent->SetWorldLocation(NewLocation);
                }
                else
                {
                    // 目标已失效，清除跟随
                    Info.FollowLocationTarget.Reset();
                }
            }
        }

        if (ShouldDestroyInstance(Info))
            InstancesToDestroy.Add(Pair.Key);
    }

    for (const FName& ID : InstancesToDestroy)
    {
        HandleEffectFinished(ID);
        DestroyEffectInstance(ID);
    }
}

void UEffectManager::HandleEffectFinished(const FName& InstanceID)
{
    const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (InstanceInfo)
    {
        OnEffectFinished.Broadcast(InstanceID, InstanceInfo->EffectID);
        if (FOnEffectFinishedCallback* Callback = EffectFinishedCallbacks.Find(InstanceID))
        {
            if (Callback->IsBound()) Callback->Execute(InstanceID, InstanceInfo->EffectID);
            EffectFinishedCallbacks.Remove(InstanceID);
        }
    }
}

void UEffectManager::HandleEffectDestroyed(const FName& InstanceID)
{
    const FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    FName EffectID = InstanceInfo ? InstanceInfo->EffectID : NAME_None;
    OnEffectDestroyed.Broadcast(InstanceID, EffectID);
    if (FOnEffectDestroyedCallback* Callback = EffectDestroyedCallbacks.Find(InstanceID))
    {
        if (Callback->IsBound()) Callback->Execute(InstanceID, EffectID);
        EffectDestroyedCallbacks.Remove(InstanceID);
    }
}

void UEffectManager::HandleDelayedActivation(FName InstanceID)
{
    DelayActivationTimers.Remove(InstanceID);
    ActivateEffectInstance(InstanceID);
}

bool UEffectManager::ShouldDestroyInstance(const FEffectInstanceInfo& InstanceInfo) const
{
    if (!InstanceInfo.EffectComponent || !IsValid(InstanceInfo.EffectComponent)) return true;
    if (InstanceInfo.TempActor && !IsValid(InstanceInfo.TempActor)) return true;
    if (InstanceInfo.ParentActor && !IsValid(InstanceInfo.ParentActor)) return true;
    if (!InstanceInfo.bIsActive || !InstanceInfo.bIsPlaying) return false;

    switch (InstanceInfo.EffectConfig.DestroyCondition)
    {
    case EEffectDestroyCondition::AfterLifetime: return InstanceInfo.RemainingLifetime <= 0.0f;
    case EEffectDestroyCondition::AfterLoops: return InstanceInfo.RemainingLoops <= 0;
    case EEffectDestroyCondition::OnFinish: return InstanceInfo.EffectComponent && !InstanceInfo.EffectComponent->IsActive();
    default: return false;
    }
}

void UEffectManager::InitializeInstanceInfo(FEffectInstanceInfo& InstanceInfo)
{
    switch (InstanceInfo.EffectConfig.DestroyCondition)
    {
    case EEffectDestroyCondition::AfterLoops:
        InstanceInfo.RemainingLoops = FMath::Max(InstanceInfo.EffectConfig.LoopCount, 1);
        break;
    case EEffectDestroyCondition::AfterLifetime:
        InstanceInfo.RemainingLifetime = FMath::Max(InstanceInfo.EffectConfig.Lifetime, 0.01f);
        break;
    default: break;
    }
}

void UEffectManager::HandlePlayTiming(const FName& InstanceID, const FEffectTableRow& Config)
{
    FEffectInstanceInfo* InstanceInfo = EffectInstances.Find(InstanceID);
    if (!InstanceInfo || !InstanceInfo->EffectComponent) return;

    UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld()) return;

    if (Config.PlayTiming == EEffectPlayTiming::Delayed && Config.DelayTime > 0.0f)
    {
        InstanceInfo->EffectComponent->SetActive(false, false);
        InstanceInfo->bIsActive = false;
        InstanceInfo->bIsPlaying = false;
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateWeakLambda(this, [this, InstanceID]() {
            if (this) HandleDelayedActivation(InstanceID);
            }), Config.DelayTime, false);
        DelayActivationTimers.Add(InstanceID, TimerHandle);
    }
    else
    {
        if (Config.bAutoActivate)
        {
            InstanceInfo->EffectComponent->Activate(true);
            InstanceInfo->bIsActive = true;
            InstanceInfo->bIsPlaying = true;
            InstanceInfo->StartTime = World->GetTimeSeconds();
        }
    }
}

FName UEffectManager::GenerateInstanceID() const
{
    static int32 NextID = 1;
    return FName(*FString::Printf(TEXT("Effect_%d"), NextID++));
}

void UEffectManager::CleanupDestroyedEffects()
{
    if (bIsCleaningUp) return;
    TArray<FName> InstancesToRemove;
    for (const auto& Pair : EffectInstances)
    {
        bool bInvalid = !Pair.Value.EffectComponent || !IsValid(Pair.Value.EffectComponent) ||
            (Pair.Value.TempActor && !IsValid(Pair.Value.TempActor)) ||
            (Pair.Value.ParentActor && !IsValid(Pair.Value.ParentActor));
        if (bInvalid) InstancesToRemove.Add(Pair.Key);
    }
    for (const FName& ID : InstancesToRemove) DestroyEffectInstance(ID);
}

UWorld* UEffectManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::GamePreview)
                if (Context.World() && Context.World()->IsGameWorld())
                    return Context.World();
        }
    }
    return nullptr;
}