#include "Character/OldManPersonPlayerController.h"
#include "Character/OldManCharacter.h"
#include "Character/OldManCameraComponent.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"

// Enhanced Input ���ͷ�ļ�
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

AOldManPersonPlayerController::AOldManPersonPlayerController()
{
	// ��ʼ������
	CachedOldManCharacter = nullptr;
	EnhancedInputComponent = nullptr;
	CurrentCameraMode = TEXT("ThirdPerson");
}

void AOldManPersonPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ��������ģʽ
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	// ע���¼�����
	RegisterEventListeners();
}

void AOldManPersonPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// ��ȡEnhanced Input���
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

	// �����ɫָ��
	CachedOldManCharacter = Cast<AOldManCharacter>(InPawn);
}

void AOldManPersonPlayerController::OnUnPossess()
{
	// �������ӳ��������
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}

	// �������
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

	// �������ӳ�������ĵ����������ϵͳ
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// ���ƶ����붯��
	if (MoveAction)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleMove);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AOldManPersonPlayerController::HandleMoveCancel);
	}

	if (LookAction)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOldManPersonPlayerController::HandleLook);
	}

	// �󶨽�ɫ��������
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

	// ������������붯��
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

// ========== �ƶ����봦���� ==========
void AOldManPersonPlayerController::HandleLook(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	// ��ȡ��ά��������ֵ�����/�ֱ��ӽ����룩
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LookAxisVector.IsNearlyZero())
		return;

	UOldManCameraComponent* CameraComp = GetCameraComponent();
	if (!CameraComp)
		return;

	// �����ӽ���ת��ͨ����������������
	float mouseSensitivity = CachedOldManCharacter ? CachedOldManCharacter->CharacterAttributes->MouseSensitivity : 1.0f;
	CameraComp->SetCameraInput(LookAxisVector.Y * mouseSensitivity, LookAxisVector.X * mouseSensitivity);
}

void AOldManPersonPlayerController::HandleMove(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter || !CachedOldManCharacter->IsAlive())
		return;

	FVector2D InputVal2D = Value.Get<FVector2D>();
	FVector InputVal = FVector(InputVal2D.X, InputVal2D.Y, 0.0f);

	// �����ƶ�����
	CachedOldManCharacter->SetMovementInput(InputVal);
}

void AOldManPersonPlayerController::HandleMoveCancel(const FInputActionValue& Value)
{
	if (!bInputEnabled || !CachedOldManCharacter)
		return;

	// ����ƶ�����
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

// ========== ����������봦���� ==========

void AOldManPersonPlayerController::HandleZoomIn(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	ZoomCamera(-50.0f); // ��С���루������
}

void AOldManPersonPlayerController::HandleZoomOut(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	ZoomCamera(50.0f); // ������루��Զ��
}

void AOldManPersonPlayerController::HandleCameraMode(const FInputActionValue& Value)
{
	if (!bInputEnabled)
		return;

	// �л����ģʽ
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

// ========== ������ƺ��� ==========

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

	// ��ȡ��ǰ���벢Ӧ������
	float CurrentDistance = CameraComp->CameraDistance;
	float NewDistance = FMath::Clamp(CurrentDistance + Delta, 100.0f, 1000.0f);
	CameraComp->SetCameraDistance(NewDistance);
}

// ========== �¼����� ==========

void AOldManPersonPlayerController::RegisterEventListeners()
{
	Super::RegisterEventListeners();

	UMyEventManager* EventMgr = GetEventManager();
	if (EventMgr)
	{
		// ע���ɫ�¼�����
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
		// �Ƴ��¼�����
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
		// �����ɫ����
		UE_LOG(LogTemp, Log, TEXT("OldMan character spawned"));
		break;
	default:
		// ���������Զ����¼�
		break;
	}
}

void AOldManPersonPlayerController::HandleCharacterDeath()
{
	// ��ɫ����ʱ��������
	SetInputEnabled(false);

	UE_LOG(LogTemp, Log, TEXT("OldMan character died, respawning in 3 seconds..."));

	// ����������ʱ��
	UMonoManager* MonoMgr = GetMonoManager();
	if (MonoMgr)
	{
		MonoMgr->SetTimeout(3.0f, this, &AOldManPersonPlayerController::HandleCharacterRespawn);
	}
}

void AOldManPersonPlayerController::HandleCharacterRespawn()
{
	// ��������
	SetInputEnabled(true);

	// ���������������߼��������������ɽ�ɫ��
	UE_LOG(LogTemp, Log, TEXT("Respawning OldMan character..."));

	// ���������¼�
	if (CachedOldManCharacter)
	{
		CachedOldManCharacter->TriggerSimpleCharacterEvent(EGameEventType::Custom, TEXT("CharacterRespawned"));
	}
}