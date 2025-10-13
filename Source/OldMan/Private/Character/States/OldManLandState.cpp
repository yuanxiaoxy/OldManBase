#include "Character/States/OldManLandState.h"
#include "Character/OldManCharacter.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManDeadState.h"

void UOldManLandState::Enter()
{
	UE_LOG(LogTemp, Log, TEXT("Entering Land State"));

	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		LandStartTime = GetWorld()->GetTimeSeconds();
		LandDuration = 0.3f; // ��ض�������ʱ��

		// ������ض���
		Character->PlayLandAnimation();

		// ������ر�־
		Character->bJustLanded = false;
	}
}

void UOldManLandState::Exit()
{
	UE_LOG(LogTemp, Log, TEXT("Exiting Land State"));
}

void UOldManLandState::Update(float DeltaTime)
{
	if (AOldManCharacter* Character = Cast<AOldManCharacter>(Owner.GetObject()))
	{
		CheckStateTransitions(Character);
	}
}

void UOldManLandState::CheckStateTransitions(AOldManCharacter* Character)
{
	if (!Character->IsAlive())
	{
		CheckTransition(UOldManDeadState::StaticClass());
		return;
	}

	// ��ض�������������ƶ�״̬ת��
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LandStartTime >= LandDuration)
	{
		if (!Character->IsMoving())
		{
			CheckTransition(UOldManIdleState::StaticClass());
		}
		else if (Character->bIsRunning)
		{
			CheckTransition(UOldManRunningState::StaticClass());
		}
		else
		{
			CheckTransition(UOldManWalkingState::StaticClass());
		}
	}
}