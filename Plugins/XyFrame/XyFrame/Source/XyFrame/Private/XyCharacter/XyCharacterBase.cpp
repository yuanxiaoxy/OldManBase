#include "XyCharacter/XyCharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"

AXyCharacterBase::AXyCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    CurrentState = EXyCharacterState::Idle;

    // ����Ĭ����ײ
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AXyCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    // ������ɫ�����¼�
    TriggerSimpleCharacterEvent(EGameEventType::PlayerSpawned, GetName(), CharacterData.Health);

    // �Զ����ؽ�ɫ��Դ
    LoadCharacterResources();
}

void AXyCharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // ������Դ
    UnloadCharacterResources();

    // ȡ�����ж�ʱ��
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

    // �����������¼�
    FGameEventData EventData;
    EventData.Actors.Add(NewController);
    TriggerCharacterEvent(EGameEventType::Custom, EventData);
}

void AXyCharacterBase::UnPossessed()
{
    Super::UnPossessed();

    // ����ʧȥ�����¼�
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

    // �����ƶ��ٶ�
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

    // �˳���״̬
    OnExitState(OldState);

    // ����״̬
    CurrentState = NewState;

    // ������״̬
    OnEnterState(NewState);

    // ����״̬�ı��¼�
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

    // �����˺��¼�
    FGameEventData EventData;
    EventData.Values.Add(Damage);
    EventData.Values.Add(CharacterData.Health);
    EventData.Actors.Add(DamageCauser);
    TriggerCharacterEvent(EGameEventType::Custom, EventData);

    // �������
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

    // ���������¼�
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
        // �޸�ί�а󶨷�ʽ - ʹ����ȷ�� C++ ģ�巽��
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
        // ����״̬�߼�
        break;
    case EXyCharacterState::Moving:
        // �ƶ�״̬�߼�
        break;
    case EXyCharacterState::Jumping:
        // ��Ծ״̬�߼�
        break;
    case EXyCharacterState::Attacking:
        // ����״̬�߼�
        break;
    }
}

void AXyCharacterBase::OnExitState(EXyCharacterState OldState)
{
    // ״̬�˳��߼�
}

void AXyCharacterBase::OnUpdateState(float DeltaTime)
{
    // ״̬�����߼�
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
    // ������ײ���ƶ�
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetActorEnableCollision(false);

    // ���������¼�
    TriggerSimpleCharacterEvent(EGameEventType::PlayerDied, TEXT("CharacterDeath"), CharacterData.Health);

    // ����������ʱ��
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
    {
        RespawnTimerId = MonoMgr->SetTimeout(5.0f, this, &AXyCharacterBase::HandleRespawn);
    }
}

void AXyCharacterBase::HandleRespawn()
{
    // ��������ֵ
    CharacterData.Health = CharacterData.MaxHealth;

    // ������ײ���ƶ�
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetActorEnableCollision(true);

    // �ص�����״̬
    ChangeState(EXyCharacterState::Idle);

    // ���������¼�
    TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterRespawned"), CharacterData.Health);
}

void AXyCharacterBase::InternalUpdateState(float DeltaTime)
{
    // ���ݽ�ɫ�ƶ�״̬����״̬��
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