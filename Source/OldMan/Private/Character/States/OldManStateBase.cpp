// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/States/OldManStateBase.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/OldManCharacter.h"
#include "EventManager/MyEventManager.h"

void UOldManStateBase::Enter()
{
	UE_LOG(LogTemp, Display, TEXT("%s : Enter"), *this->GetName());
	InPatchEvents();
}

void UOldManStateBase::Exit()
{
	UE_LOG(LogTemp, Display, TEXT("%s : Exit"), *this->GetName());
	OutPatchEvents();
}

void UOldManStateBase::CheckJumpStatesTranisition()
{
	//if (CanDoubleJump())
	//{
	//	//CheckTransition(UOldManDoubleJumpingState::StaticClass());
	//	return;
	//}
	//else
	//{
	//	StateMachine->ChangeState(UOldManJumpingState::StaticClass());
	//}
}

void UOldManStateBase::CheckAttackStatesTranisition()
{
}

void UOldManStateBase::InPatchEvents()
{
	UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckJumpStatesTranisition, this, &UOldManStateBase::CheckJumpStatesTranisition);
	UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckAttackStatesTranisition, this, &UOldManStateBase::CheckAttackStatesTranisition);
}

void UOldManStateBase::OutPatchEvents()
{
	UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckJumpStatesTranisition, this, &UOldManStateBase::CheckJumpStatesTranisition);
	UMyEventManager::GetInstance()->RegisterCppEvent<>(Key_CheckAttackStatesTranisition, this, &UOldManStateBase::CheckAttackStatesTranisition);
}
