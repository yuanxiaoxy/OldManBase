// DraggableSplineActorManager.cpp
#include "DraggableSplineActor/DraggableSplineActorManager.h"
#include "EngineUtils.h"
#include "Engine/World.h"

ADraggableSplineActorManager::ADraggableSplineActorManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ADraggableSplineActorManager::BeginPlay()
{
    Super::BeginPlay();

    // 扫描并绑定场景中已存在的所有可拖动物体
    DiscoverExistingActors();

    UE_LOG(LogTemp, Log, TEXT("DraggableSplineActorManager initialized, managing %d actors"), ManagedActors.Num());
}

void ADraggableSplineActorManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理所有委托绑定
    for (const TWeakObjectPtr<ADraggableSplineActor>& WeakActor : ManagedActors)
    {
        if (ADraggableSplineActor* Actor = WeakActor.Get())
        {
            UnbindActorEvents(Actor);
        }
    }
    ManagedActors.Empty();

    Super::EndPlay(EndPlayReason);
}

void ADraggableSplineActorManager::DiscoverExistingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ADraggableSplineActor> It(World); It; ++It)
    {
        ADraggableSplineActor* Actor = *It;
        if (Actor && IsValid(Actor))
        {
            RegisterDraggableActor(Actor);
        }
    }
}

void ADraggableSplineActorManager::RegisterDraggableActor(ADraggableSplineActor* Actor)
{
    if (!Actor || !IsValid(Actor)) return;

    TWeakObjectPtr<ADraggableSplineActor> WeakActor(Actor);
    if (ManagedActors.Contains(WeakActor)) return;

    ManagedActors.Add(WeakActor);
    BindActorEvents(Actor);

    UE_LOG(LogTemp, Verbose, TEXT("DraggableSplineActorManager: Registered actor %s"), *Actor->GetName());
}

void ADraggableSplineActorManager::UnregisterDraggableActor(ADraggableSplineActor* Actor)
{
    if (!Actor) return;

    TWeakObjectPtr<ADraggableSplineActor> WeakActor(Actor);
    if (ManagedActors.Remove(WeakActor) > 0)
    {
        UnbindActorEvents(Actor);
        UE_LOG(LogTemp, Verbose, TEXT("DraggableSplineActorManager: Unregistered actor %s"), *Actor->GetName());
    }
}

void ADraggableSplineActorManager::BindActorEvents(ADraggableSplineActor* Actor)
{
    if (!Actor) return;
    Actor->OnDraggingStarted.AddDynamic(this, &ADraggableSplineActorManager::HandleDraggingStarted);
    Actor->OnDraggingStopped.AddDynamic(this, &ADraggableSplineActorManager::HandleDraggingStopped);
}

void ADraggableSplineActorManager::UnbindActorEvents(ADraggableSplineActor* Actor)
{
    if (!Actor) return;
    Actor->OnDraggingStarted.RemoveDynamic(this, &ADraggableSplineActorManager::HandleDraggingStarted);
    Actor->OnDraggingStopped.RemoveDynamic(this, &ADraggableSplineActorManager::HandleDraggingStopped);
}

void ADraggableSplineActorManager::HandleDraggingStarted(ADraggableSplineActor* DraggedActor)
{
    // 转发聚合事件
    OnAnyDraggingStarted.Broadcast(DraggedActor);
}

void ADraggableSplineActorManager::HandleDraggingStopped(ADraggableSplineActor* DraggedActor)
{
    // 转发聚合事件
    OnAnyDraggingStopped.Broadcast(DraggedActor);
}

TArray<ADraggableSplineActor*> ADraggableSplineActorManager::GetAllManagedActors() const
{
    TArray<ADraggableSplineActor*> Result;
    for (const TWeakObjectPtr<ADraggableSplineActor>& WeakActor : ManagedActors)
    {
        if (ADraggableSplineActor* Actor = WeakActor.Get())
        {
            Result.Add(Actor);
        }
    }
    return Result;
}

void ADraggableSplineActorManager::ResetDraggableSplineActorPos(const FString& GroupName)
{
    if (DraggableActorMap.Contains(GroupName))
    {
        for (ADraggableSplineActor* DraggableActor : DraggableActorMap[GroupName].DraggableSplineActors)
        {
            if (DraggableActor && IsValid(DraggableActor))
            {
                DraggableActor->SetStartPosition();
            }
        }
    }
}