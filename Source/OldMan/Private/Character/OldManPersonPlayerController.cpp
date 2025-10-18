#include "Character/OldManPersonPlayerController.h"
#include "Character/OldManCharacter.h"
#include "Character/OldManCameraComponent.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"

// Enhanced Input 相关头文件
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AOldManPersonPlayerController::AOldManPersonPlayerController()
{
	// 初始化变量
	CachedOldManCharacter = nullptr;
	EnhancedInputComponent = nullptr;
	CurrentCameraMode = TEXT("ThirdPerson");
}

void AOldManPersonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 设置输入模式
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	// 注册事件监听
	RegisterEventListeners();
}

void AOldManPersonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// 获取Enhanced Input组件
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get EnhancedInputComponent"));
		return;
	}
}

void AOldManPersonPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 缓存角色指针
	CachedOldManCharacter = Cast<AOldManCharacter>(InPawn);
}

void AOldManPersonPlayerController::OnUnPossess()
{
	// 清除输入映射上下文
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}

	// 清除缓存
	CachedOldManCharacter = nullptr;
	EnhancedInputComponent = nullptr;

	Super::OnUnPossess();
}

void AOldManPersonPlayerController::BindCharacterInputs()
{
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent is null in BindCharacterInputs"));
		return;
	}

	if (!DefaultMappingContext)
	{
		UE_LOG(LogTemp, Error, TEXT("DefaultMappingContext is not set in OldManPersonPlayerController"));
		return;
	}

	// 添加输入映射上下文到本地玩家子系统
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// 绑定移动输入动作
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleMoveCancel);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleLook);
	}

	// 绑定角色动作输入
	if (JumpAction)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleJumpStop);
	}

	if (RunAction)
	{
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleStartRunning);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleStopRunning);
	}

	if (AttackAction)
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleAttackStart);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleAttackStop);
	}

	if (RightMouseAction)
	{
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleRightMouseStart);
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleRightMouseStop);

	}

	// 绑定相机控制输入动作
	if (ZoomInAction)
	{
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleZoomIn);
	}

	if (ZoomOutAction)
	{
		EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleZoomOut);
	}

	if (CameraModeAction)
	{
		EnhancedInputComponent->BindAction(CameraModeAction, ETriggerEvent::Started, this, &AOldManPersonPlayerController::HandleCameraMode);
	}

	UE_LOG(LogTemp, Log, TEXT("Enhanced Input bindings setup for OldMan character and camera"));
}

// ========== 移动输入处理函数 ==========
void AOldManPersonPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	// 获取二维向量输入值（鼠标/手柄视角输入）
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LookAxisVector.IsNearlyZero())
		return;

	UOldManCameraComponent* CameraComp = GetCameraComponent();
	if (!CameraComp)
		return;

	// 处理视角旋转，通过相机组件传递输入
	float mouseSensitivity = CachedOldManCharacter ? CachedOldManCharacter->CharacterAttributes->MouseSensitivity : 1.0f;
	CameraComp->SetCameraInput(LookAxisVector.Y * mouseSensitivity, LookAxisVector.X * mouseSensitivity);
}

void AOldManPersonPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	FVector2D InputVal2D = Value.Get<FVector2D>();
	FVector InputVal = FVector(InputVal2D.X, InputVal2D.Y, 0.0f);

	// 设置移动输入
	CachedOldManCharacter->SetMovementInput(InputVal);
}

void AOldManPersonPlayerController::HandleMoveCancel(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter)
		return;

	// 清除移动输入
	CachedOldManCharacter->SetMovementInput(FVector::ZeroVector);
}

void AOldManPersonPlayerController::HandleJumpStart(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	CachedOldManCharacter->SetJumpInput(true);
}

void AOldManPersonPlayerController::HandleJumpStop(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter)
		return;

	CachedOldManCharacter->SetJumpInput(false);
}

void AOldManPersonPlayerController::HandleAttackStart(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	CachedOldManCharacter->SetAttackInput(true);
}

void AOldManPersonPlayerController::HandleAttackStop(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter)
		return;

	CachedOldManCharacter->SetAttackInput(false);
}

void AOldManPersonPlayerController::HandleRightMouseStart(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	CachedOldManCharacter->StartRightMouseInterect();
}

void AOldManPersonPlayerController::HandleRightMouseStop(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	CachedOldManCharacter->StopRightMouseInterect();
}

void AOldManPersonPlayerController::HandleStartRunning(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	CachedOldManCharacter->SetRunning(true);
}

void AOldManPersonPlayerController::HandleStopRunning(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter)
		return;

	CachedOldManCharacter->SetRunning(false);
}

// ========== 相机控制输入处理函数 ==========

void AOldManPersonPlayerController::HandleZoomIn(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	ZoomCamera(-50.0f); // 缩小距离（拉近）
}

void AOldManPersonPlayerController::HandleZoomOut(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	ZoomCamera(50.0f); // 增大距离（拉远）
}

void AOldManPersonPlayerController::HandleCameraMode(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	// 切换相机模式
	if (CurrentCameraMode == TEXT("ThirdPerson"))
	{
		SetCameraMode(TEXT("FirstPerson"));
	}
	else if (CurrentCameraMode == TEXT("FirstPerson"))
	{
		SetCameraMode(TEXT("FreeLook"));
	}
	else
	{
		SetCameraMode(TEXT("ThirdPerson"));
	}
}

// ========== 相机控制函数 ==========

UOldManCameraComponent* AOldManPersonPlayerController::GetCameraComponent() const
{
	if (CachedOldManCharacter)
	{
		return CachedOldManCharacter->CameraComponent;
	}
	return nullptr;
}

void AOldManPersonPlayerController::SetCameraMode(FName NewMode)
{
	CurrentCameraMode = NewMode;

	UOldManCameraComponent* CameraComp = GetCameraComponent();
	if (!CameraComp)
		return;

	if (NewMode == TEXT("ThirdPerson"))
	{
		CameraComp->SetThirdPersonMode();
		UE_LOG(LogTemp, Log, TEXT("Camera mode switched to Third Person"));
	}
	else if (NewMode == TEXT("FirstPerson"))
	{
		CameraComp->SetFirstPersonMode();
		UE_LOG(LogTemp, Log, TEXT("Camera mode switched to First Person"));
	}
	else if (NewMode == TEXT("FreeLook"))
	{
		CameraComp->SetFreeLookMode();
		UE_LOG(LogTemp, Log, TEXT("Camera mode switched to Free Look"));
	}
}

void AOldManPersonPlayerController::ZoomCamera(float Delta)
{
	UOldManCameraComponent* CameraComp = GetCameraComponent();
	if (!CameraComp)
		return;

	// 获取当前距离并应用增量
	float CurrentDistance = CameraComp->CameraDistance;
	float NewDistance = FMath::Clamp(CurrentDistance + Delta, 100.0f, 1000.0f);
	CameraComp->SetCameraDistance(NewDistance);
}

// ========== 事件处理 ==========

void AOldManPersonPlayerController::RegisterEventListeners()
{
	Super::RegisterEventListeners();

	UMyEventManager* EventMgr = GetEventManager();
	if (EventMgr)
	{
		// 注册角色事件监听
		EventMgr->RegisterCppEvent<AOldManPersonPlayerController, EGameEventType, const FGameEventData&>(
			FName("OldManCharacterEvents"),
			this,
			&AOldManPersonPlayerController::OnCharacterEvent
		);
	}
}

void AOldManPersonPlayerController::UnregisterEventListeners()
{
	UMyEventManager* EventMgr = GetEventManager();
	if (EventMgr)
	{
		// 移除事件监听
		EventMgr->RemoveCppEvent(FName("OldManCharacterEvents"));
	}

	Super::UnregisterEventListeners();
}

void AOldManPersonPlayerController::OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData)
{
	switch (EventType)
	{
	case EGameEventType::PlayerDied:
		HandleCharacterDeath();
		break;
	case EGameEventType::PlayerSpawned:
		// 处理角色生成
		UE_LOG(LogTemp, Log, TEXT("OldMan character spawned"));
		break;
	default:
		// 处理其他自定义事件
		break;
	}
}

void AOldManPersonPlayerController::HandleCharacterDeath()
{
	// 角色死亡时禁用输入
	SetInputEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("OldMan character died, respawning in 3 seconds..."));

	// 设置重生定时器
	UMonoManager* MonoMgr = GetMonoManager();
	if (MonoMgr)
	{
		MonoMgr->SetTimeout(3.0f, this, &AOldManPersonPlayerController::HandleCharacterRespawn);
	}
}

void AOldManPersonPlayerController::HandleCharacterRespawn()
{
	// 启用输入
	SetInputEnabled(true);

	// 这里可以添加重生逻辑，比如重新生成角色等
	UE_LOG(LogTemp, Log, TEXT("Respawning OldMan character..."));

	// 触发重生事件
	if (CachedOldManCharacter)
	{
		CachedOldManCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterRespawned"));
	}
}