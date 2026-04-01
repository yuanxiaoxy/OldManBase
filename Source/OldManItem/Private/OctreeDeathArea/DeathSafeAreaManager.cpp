#include "OctreeDeathArea/DeathSafeAreaManager.h"

ADeathSafeAreaManager::ADeathSafeAreaManager()
{
    PrimaryActorTick.bCanEverTick = false; // 管理器组件自己会 Tick，Actor 本身不需要 Tick

    // 创建管理器组件
    ManagerComponent = CreateDefaultSubobject<UDeathSafeAreaManagerComponent>(TEXT("ManagerComponent"));
    if (ManagerComponent)
    {
        // 可选：设置默认调试显示为 true
        ManagerComponent->bDrawDebug = true;
        // 设置默认模式（通常使用默认安全）
        ManagerComponent->Mode = EOctreeMode::DefaultSafe;
    }
}

void ADeathSafeAreaManager::BeginPlay()
{
    Super::BeginPlay();

    // 绑定组件的事件，再转发给 Actor 的委托
    if (ManagerComponent)
    {
        ManagerComponent->OnAreaStateChanged.AddDynamic(this, &ADeathSafeAreaManager::OnManagerStateChanged);
    }
}

void ADeathSafeAreaManager::OnManagerStateChanged(AActor* Actor, bool bIsSafe)
{
    // 转发事件
    OnAreaStateChanged.Broadcast(Actor, bIsSafe);
}