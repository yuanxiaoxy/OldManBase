#include "XyCharacter/XyCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

AXyCharacterBase::AXyCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    CurrentState = EXyCharacterState::Idle;

    // 设置默认碰撞
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AXyCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // 触发角色生成事件
    TriggerSimpleCharacterEvent(EGameEventType::PlayerSpawned, GetName(), CharacterData.Health);

    // 自动加载角色资源
    LoadCharacterResources();
}

void AXyCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理资源
    UnloadCharacterResources();

    // 取消所有定时器
    if (!RespawnTimerId.IsEmpty())
    {
        UMonoManager* MonoMgr = GetMonoManager();
        if (MonoMgr)
        {
            MonoMgr->ClearTimer(RespawnTimerId);
        }
    }

    Super::EndPlay(EndPlayReason);
}

void AXyCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    // 触发被控制事件
    FGameEventData EventData;
    EventData.Actors.Add(NewController);
    TriggerCharacterEvent(EGameEventType::Custom, EventData);
}

void AXyCharacterBase::UnPossessed()
{
    Super::UnPossessed();

    // 触发失去控制事件
    TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterUnpossessed"));
}

void AXyCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    InternalUpdateState(DeltaTime);
    OnUpdateState(DeltaTime);
}

void AXyCharacterBase::InitializeCharacter(const FCharacterData& InData)
{
    CharacterData = InData;

    // 设置移动速度
    if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed = CharacterData.MoveSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("Character initialized: %s, Health: %.1f/%.1f"),
        *CharacterData.CharacterName, CharacterData.Health, CharacterData.MaxHealth);
}

bool AXyCharacterBase::ChangeState(EXyCharacterState NewState)
{
    if (CurrentState == NewState)
        return false;

    EXyCharacterState OldState = CurrentState;

    // 退出旧状态
    OnExitState(OldState);

    // 更新状态
    CurrentState = NewState;

    // 进入新状态
    OnEnterState(NewState);

    // 触发状态改变事件
    FGameEventData EventData;
    EventData.Values.Add(static_cast<float>(NewState));
    EventData.Texts.Add(UEnum::GetValueAsString(NewState));
    TriggerCharacterEvent(EGameEventType::Custom, EventData);

    return true;
}

void AXyCharacterBase::ApplyDamage(float Damage, AActor* DamageCauser)
{
    if (!IsAlive() || Damage <= 0)
        return;

    CharacterData.Health = FMath::Max(0.0f, CharacterData.Health - Damage);

    // 触发伤害事件
    FGameEventData EventData;
    EventData.Values.Add(Damage);
    EventData.Values.Add(CharacterData.Health);
    EventData.Actors.Add(DamageCauser);
    TriggerCharacterEvent(EGameEventType::Custom, EventData);

    // 检查死亡
    if (CharacterData.Health <= 0)
    {
        ChangeState(EXyCharacterState::Dead);
    }
}

void AXyCharacterBase::ApplyHeal(float HealAmount)
{
    if (!IsAlive() || HealAmount <= 0)
        return;

    CharacterData.Health = FMath::Min(CharacterData.MaxHealth, CharacterData.Health + HealAmount);

    // 触发治疗事件
    FGameEventData EventData;
    EventData.Values.Add(HealAmount);
    EventData.Values.Add(CharacterData.Health);
    TriggerCharacterEvent(EGameEventType::Custom, EventData);
}

void AXyCharacterBase::LoadCharacterResources()
{
    if (CharacterMeshPath.IsEmpty())
        return;

    UResourceManager* ResourceMgr = GetResourceManager();
    if (ResourceMgr)
    {
        // 修复委托绑定方式 - 使用正确的 C++ 模板方法
        ResourceMgr->LoadResourceAsyncWithCallback(
            CharacterMeshPath,
            this,
            &AXyCharacterBase::OnCharacterResourcesLoaded
        );
    }
}

void AXyCharacterBase::UnloadCharacterResources()
{
    if (!ResourceLoadRequestId.IsEmpty())
    {
        UResourceManager* ResourceMgr = GetResourceManager();
        if (ResourceMgr)
        {
            ResourceMgr->CancelAsyncRequest(ResourceLoadRequestId);
        }
    }
}

void AXyCharacterBase::TriggerCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr)
    {
        EventMgr->TriggerGameEvent(EventType, EventData);
    }
}

void AXyCharacterBase::TriggerSimpleCharacterEvent(EGameEventType EventType, const FString& TextParam, float ValueParam)
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr)
    {
        EventMgr->TriggerSimpleGameEvent(EventType, TextParam, ValueParam, this);
    }
}

void AXyCharacterBase::OnEnterState(EXyCharacterState NewState)
{
    switch (NewState)
    {
    case EXyCharacterState::Dead:
        HandleDeath();
        break;
    case EXyCharacterState::Idle:
        // 闲置状态逻辑
        break;
    case EXyCharacterState::Moving:
        // 移动状态逻辑
        break;
    case EXyCharacterState::Jumping:
        // 跳跃状态逻辑
        break;
    case EXyCharacterState::Attacking:
        // 攻击状态逻辑
        break;
    }
}

void AXyCharacterBase::OnExitState(EXyCharacterState OldState)
{
    // 状态退出逻辑
}

void AXyCharacterBase::OnUpdateState(float DeltaTime)
{
    // 状态更新逻辑
}

void AXyCharacterBase::OnCharacterResourcesLoaded(UObject* LoadedResource)
{
    if (USkeletalMesh* CharacterMesh = Cast<USkeletalMesh>(LoadedResource))
    {
        GetMesh()->SetSkeletalMesh(CharacterMesh);
        UE_LOG(LogTemp, Log, TEXT("Character mesh loaded: %s"), *GetNameSafe(CharacterMesh));
    }
}

void AXyCharacterBase::HandleDeath()
{
    // 禁用碰撞和移动
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorEnableCollision(false);

    // 触发死亡事件
    TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("CharacterDeath"), CharacterData.Health);

    // 设置重生定时器
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
    {
        RespawnTimerId = MonoMgr->SetTimeout(5.0f, this, &AXyCharacterBase::HandleRespawn);
    }
}

void AXyCharacterBase::HandleRespawn()
{
    // 重置生命值
    CharacterData.Health = CharacterData.MaxHealth;

    // 启用碰撞和移动
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorEnableCollision(true);

    // 回到闲置状态
    ChangeState(EXyCharacterState::Idle);

    // 触发重生事件
    TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterRespawned"), CharacterData.Health);
}

void AXyCharacterBase::InternalUpdateState(float DeltaTime)
{
    // 根据角色移动状态更新状态机
    if (CurrentState != EXyCharacterState::Dead)
    {
        UCharacterMovementComponent* Movement = GetCharacterMovement();
        if (Movement)
        {
            if (Movement->IsFalling())
            {
                if (CurrentState != EXyCharacterState::Jumping)
                {
                    ChangeState(EXyCharacterState::Jumping);
                }
            }
            else if (Movement->Velocity.SizeSquared() > 0.1f)
            {
                if (CurrentState != EXyCharacterState::Moving && CurrentState != EXyCharacterState::Attacking)
                {
                    ChangeState(EXyCharacterState::Moving);
                }
            }
            else
            {
                if (CurrentState != EXyCharacterState::Idle && CurrentState != EXyCharacterState::Attacking)
                {
                    ChangeState(EXyCharacterState::Idle);
                }
            }
        }
    }
}