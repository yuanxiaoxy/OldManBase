#include "SavePoint/OldManSavePointManager.h"
#include "GlobalEventName.h"
#include "EventManager/MyEventManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "XyGameModeBase/XyBaseGameMode.h"
#include "SavePoint/OldManSavePoint.h"
#include "GameFramework/Character.h"

// 静态实例定义
template<>
UOldManSavePointManager* TSingleton<UOldManSavePointManager>::SingletonInstance = nullptr;

void UOldManSavePointManager::InitializeSingleton()
{
	UMyEventManager::GetInstance()->RegisterCppEvent<UOldManSavePointManager, float>(UGlobalEventName::Key_Player_OnDeath, this, &UOldManSavePointManager::OnPlayerDead);
}

UOldManSavePointManager::UOldManSavePointManager()
{

}

UOldManSavePointManager::~UOldManSavePointManager()
{

}

void UOldManSavePointManager::SetNewActivePoint(AOldManSavePoint* NewActiveSavePoint)
{
	if (ActiveSavePointArray.Contains(NewActiveSavePoint))
	{
		return;
	}

	CurActiveSavePoint = NewActiveSavePoint;
	ActiveSavePointArray.Add(NewActiveSavePoint);

	FGameEventData EventData;
	EventData.Actors.Add(CurActiveSavePoint);
	UMyEventManager::GetInstance()->TriggerEvent(UGlobalEventName::Key_Save_SavePointActive, EventData);
}

void UOldManSavePointManager::OnPlayerDead(float RebornTime)
{
	//UMonoManager::GetInstance()->SetTimeout(RebornTime, this, &UOldManSavePointManager::OnPlayerRespawn);
}

void UOldManSavePointManager::RestartPlayer()
{
	OnPlayerRespawn();
}

void UOldManSavePointManager::OnPlayerRespawn()
{
	if (ACharacter* player =  UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		bool ShouldResetCamera = false;
		bool ShouldWaitFotInput = false;
		FVector ReBornPosition = FVector::ZeroVector;
		FRotator ReBornRotation = FRotator::ZeroRotator;

		if (CurActiveSavePoint)
		{
			ReBornPosition = CurActiveSavePoint->RebornPosition->GetComponentLocation();
			ReBornRotation = CurActiveSavePoint->RebornPosition->GetComponentRotation();
			ShouldWaitFotInput = CurActiveSavePoint->IfRebornWaitForInput;
			ShouldResetCamera = CurActiveSavePoint->IfResetCamera;
		}
		else
		{
			UWorld* World = GetWorld();
			APlayerController* PlayerController = World->GetFirstPlayerController();
			AGameModeBase* GameMode = UGameplayStatics::GetGameMode(World);
			if (PlayerController && GameMode)
			{
				// 调用 GameMode 的 FindPlayerStart 方法
				AActor* PlayerStart = GameMode->FindPlayerStart(PlayerController);
				if (PlayerStart)
				{
					ReBornPosition = PlayerStart->GetActorLocation();
					ReBornRotation = PlayerStart->GetActorRotation();
				}
				else
				{
					if (AXyBaseGameMode* XyGameMode = Cast<AXyBaseGameMode>(GameMode))
					{
						XyGameMode->RestartWorld();
					}
				}

			}
		}

		UMyEventManager::GetInstance()->TriggerCppEvent<bool>(UGlobalEventName::Key_Player_OnRespawn, ShouldWaitFotInput, ReBornPosition, ReBornRotation, ShouldResetCamera);
		FGameEventData tempEventData;
		UMyEventManager::GetInstance()->TriggerEventString(UGlobalEventName::Key_Player_OnRespawn.ToString(), tempEventData);
	}
}

UWorld* UOldManSavePointManager::GetWorld() const
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
