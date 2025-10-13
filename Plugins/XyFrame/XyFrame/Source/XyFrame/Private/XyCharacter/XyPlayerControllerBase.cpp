#include "XyCharacter/XyPlayerControllerBase.h"
#include "XyCharacter/XyCharacterBase.h"
#include "Engine/Engine.h"

AXyPlayerControllerBase::AXyPlayerControllerBase()
{
    PrimaryActorTick.bCanEverTick = true;

    bInputEnabled = true;
    MouseSensitivity = 1.0f;
    ControllerSensitivity = 1.0f;
    CachedInputComponent = nullptr;
    CachedXyCharacter = nullptr;
    bCachedCharacterValid = false;  // ������
}

void AXyPlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();

    // ��������ģʽ
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    // ע���¼�����
    RegisterEventListeners();

    UE_LOG(LogTemp, Log, TEXT("XyPlayerControllerBase BeginPlay"));
}

void AXyPlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // �Ƴ��¼�����
    UnregisterEventListeners();

    Super::EndPlay(EndPlayReason);
}

void AXyPlayerControllerBase::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // ���»���Ľ�ɫָ��
    CachedXyCharacter = Cast<AXyCharacterBase>(InPawn);
    bCachedCharacterValid = IsValid(CachedXyCharacter);  // ������Ч�Ա�־

    // ���½�ɫ������
    BindCharacterInputs();

    // ���������¼�
    if (bCachedCharacterValid)
    {
        CachedXyCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterPossessed"));
    }
}

void AXyPlayerControllerBase::OnUnPossess()
{
    // ����ʧȥ�����¼�
    if (bCachedCharacterValid)
    {
        CachedXyCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterUnpossessed"));
    }

    // �������
    CachedXyCharacter = nullptr;
    bCachedCharacterValid = false;  // ���ñ�־

    Super::OnUnPossess();
}

void AXyPlayerControllerBase::SetupInputComponent()
{
    Super::SetupInputComponent();

    CachedInputComponent = InputComponent;
    BindCharacterInputs();
}

void AXyPlayerControllerBase::BindCharacterInputs()
{
    if (!CachedInputComponent)
        return;

    // ������а�
    CachedInputComponent->ClearActionBindings();
    CachedInputComponent->AxisBindings.Empty();

    // ��������
    CachedInputComponent->BindAxis("MoveForward", this, &AXyPlayerControllerBase::HandleMoveForward);
    CachedInputComponent->BindAxis("MoveRight", this, &AXyPlayerControllerBase::HandleMoveRight);
    CachedInputComponent->BindAxis("LookUp", this, &AXyPlayerControllerBase::HandleLookUp);
    CachedInputComponent->BindAxis("Turn", this, &AXyPlayerControllerBase::HandleTurn);

    // �󶨶�������
    CachedInputComponent->BindAction("Jump", IE_Pressed, this, &AXyPlayerControllerBase::HandleJump);
    CachedInputComponent->BindAction("Jump", IE_Released, this, &AXyPlayerControllerBase::HandleStopJumping);
    CachedInputComponent->BindAction("Attack", IE_Pressed, this, &AXyPlayerControllerBase::HandleAttack);

    UE_LOG(LogTemp, Log, TEXT("Character inputs bound"));
}

void AXyPlayerControllerBase::SetInputEnabled(bool bEnabled)
{
    bInputEnabled = bEnabled;

    if (bInputEnabled)
    {
        // ��������
        GetPawn()->EnableInput(this);
    }
    else
    {
        // ��������
        GetPawn()->DisableInput(this);
    }
}

// �Ż��� GetXyCharacter ����
AXyCharacterBase* AXyPlayerControllerBase::GetXyCharacter() const
{
    // ���������Ч��ֱ�ӷ���
    if (IsValid(CachedXyCharacter))
    {
        return CachedXyCharacter;
    }

    // ������Чʱ���»�ȡ
    CachedXyCharacter = Cast<AXyCharacterBase>(GetPawn());
    return CachedXyCharacter;
}

void AXyPlayerControllerBase::RespawnCharacter()
{
    if (GetXyCharacter() && !GetXyCharacter()->IsAlive())
    {
        // �����߼�
    }
}

void AXyPlayerControllerBase::RegisterEventListeners()
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr)
    {
        // ע��C++�¼�����
        EventMgr->RegisterCppEvent<AXyPlayerControllerBase, EGameEventType, const FGameEventData&>(
            FName("CharacterEvents"),
            this,
            &AXyPlayerControllerBase::OnCharacterEvent
        );
    }
}

void AXyPlayerControllerBase::UnregisterEventListeners()
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr)
    {
        // �Ƴ��¼�����
        EventMgr->RemoveCppEvent(FName("CharacterEvents"));
    }
}

// ========== ���봦��ʵ�� ==========

void AXyPlayerControllerBase::HandleMoveForward(float Value)
{
    if (!bInputEnabled || Value == 0.0f)
        return;

    AXyCharacterBase* ControlledCharacter = GetXyCharacter();
    if (ControlledCharacter && ControlledCharacter->IsAlive())
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        ControlledCharacter->AddMovementInput(Direction, Value);
    }
}

void AXyPlayerControllerBase::HandleMoveRight(float Value)
{
    if (!bInputEnabled || Value == 0.0f)
        return;

    AXyCharacterBase* ControlledCharacter = GetXyCharacter();
    if (ControlledCharacter && ControlledCharacter->IsAlive())
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        ControlledCharacter->AddMovementInput(Direction, Value);
    }
}

void AXyPlayerControllerBase::HandleLookUp(float Value)
{
    if (!bInputEnabled || Value == 0.0f)
        return;

    AddPitchInput(Value * MouseSensitivity);
}

void AXyPlayerControllerBase::HandleTurn(float Value)
{
    if (!bInputEnabled || Value == 0.0f)
        return;

    AddYawInput(Value * MouseSensitivity);
}

void AXyPlayerControllerBase::HandleJump()
{
    if (!bInputEnabled)
        return;

    AXyCharacterBase* ControlledCharacter = GetXyCharacter();
    if (ControlledCharacter && ControlledCharacter->IsAlive())
    {
        ControlledCharacter->Jump();
    }
}

void AXyPlayerControllerBase::HandleStopJumping()
{
    if (!bInputEnabled)
        return;

    AXyCharacterBase* ControlledCharacter = GetXyCharacter();
    if (ControlledCharacter)
    {
        ControlledCharacter->StopJumping();
    }
}

void AXyPlayerControllerBase::HandleAttack()
{
    if (!bInputEnabled)
        return;

    AXyCharacterBase* ControlledCharacter = GetXyCharacter();
    if (ControlledCharacter && ControlledCharacter->IsAlive())
    {
        // ���������¼�
        ControlledCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterAttack"));

        // ���������չ�����߼�
        // ControlledCharacter->ChangeState(EXyCharacterState::Attacking);
    }
}

// ========== �¼��ص� ==========

void AXyPlayerControllerBase::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    switch (EventType)
    {
    case EGameEventType::PlayerDied:
        HandleCharacterDeath();
        break;
    case EGameEventType::PlayerSpawned:
        // �����ɫ����
        break;
    case EGameEventType::ItemCollected:
        // ������Ʒ�ռ�
        break;
    default:
        // ���������Զ����¼�
        break;
    }
}

void AXyPlayerControllerBase::HandleCharacterDeath()
{
    // ��ɫ����ʱ��������
    SetInputEnabled(false);

    // ����������ʱ��
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
    {
        MonoMgr->SetTimeout(3.0f, this, &AXyPlayerControllerBase::RespawnCharacter);
    }
}