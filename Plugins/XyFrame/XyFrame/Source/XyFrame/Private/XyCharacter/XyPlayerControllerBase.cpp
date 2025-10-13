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
    bCachedCharacterValid = false;  // 添加这个
}

void AXyPlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();

    // 设置输入模式
    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

    // 注册事件监听
    RegisterEventListeners();

    UE_LOG(LogTemp, Log, TEXT("XyPlayerControllerBase BeginPlay"));
}

void AXyPlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 移除事件监听
    UnregisterEventListeners();

    Super::EndPlay(EndPlayReason);
}

void AXyPlayerControllerBase::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 更新缓存的角色指针
    CachedXyCharacter = Cast<AXyCharacterBase>(InPawn);
    bCachedCharacterValid = IsValid(CachedXyCharacter);  // 设置有效性标志

    // 绑定新角色的输入
    BindCharacterInputs();

    // 触发控制事件
    if (bCachedCharacterValid)
    {
        CachedXyCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterPossessed"));
    }
}

void AXyPlayerControllerBase::OnUnPossess()
{
    // 触发失去控制事件
    if (bCachedCharacterValid)
    {
        CachedXyCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterUnpossessed"));
    }

    // 清除缓存
    CachedXyCharacter = nullptr;
    bCachedCharacterValid = false;  // 重置标志

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

    // 清除现有绑定
    CachedInputComponent->ClearActionBindings();
    CachedInputComponent->AxisBindings.Empty();

    // 绑定轴输入
    CachedInputComponent->BindAxis("MoveForward", this, &AXyPlayerControllerBase::HandleMoveForward);
    CachedInputComponent->BindAxis("MoveRight", this, &AXyPlayerControllerBase::HandleMoveRight);
    CachedInputComponent->BindAxis("LookUp", this, &AXyPlayerControllerBase::HandleLookUp);
    CachedInputComponent->BindAxis("Turn", this, &AXyPlayerControllerBase::HandleTurn);

    // 绑定动作输入
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
        // 启用输入
        GetPawn()->EnableInput(this);
    }
    else
    {
        // 禁用输入
        GetPawn()->DisableInput(this);
    }
}

// 优化的 GetXyCharacter 方法
AXyCharacterBase* AXyPlayerControllerBase::GetXyCharacter() const
{
    // 如果缓存有效，直接返回
    if (IsValid(CachedXyCharacter))
    {
        return CachedXyCharacter;
    }

    // 缓存无效时重新获取
    CachedXyCharacter = Cast<AXyCharacterBase>(GetPawn());
    return CachedXyCharacter;
}

void AXyPlayerControllerBase::RespawnCharacter()
{
    if (GetXyCharacter() && !GetXyCharacter()->IsAlive())
    {
        // 重生逻辑
    }
}

void AXyPlayerControllerBase::RegisterEventListeners()
{
    UMyEventManager* EventMgr = GetEventManager();
    if (EventMgr)
    {
        // 注册C++事件监听
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
        // 移除事件监听
        EventMgr->RemoveCppEvent(FName("CharacterEvents"));
    }
}

// ========== 输入处理实现 ==========

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
        // 触发攻击事件
        ControlledCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterAttack"));

        // 这里可以扩展攻击逻辑
        // ControlledCharacter->ChangeState(EXyCharacterState::Attacking);
    }
}

// ========== 事件回调 ==========

void AXyPlayerControllerBase::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
    switch (EventType)
    {
    case EGameEventType::PlayerDied:
        HandleCharacterDeath();
        break;
    case EGameEventType::PlayerSpawned:
        // 处理角色生成
        break;
    case EGameEventType::ItemCollected:
        // 处理物品收集
        break;
    default:
        // 处理其他自定义事件
        break;
    }
}

void AXyPlayerControllerBase::HandleCharacterDeath()
{
    // 角色死亡时禁用输入
    SetInputEnabled(false);

    // 设置重生定时器
    UMonoManager* MonoMgr = GetMonoManager();
    if (MonoMgr)
    {
        MonoMgr->SetTimeout(3.0f, this, &AXyPlayerControllerBase::RespawnCharacter);
    }
}