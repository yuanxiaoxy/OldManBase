// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GlobalEventName.generated.h"

UCLASS(Blueprintable)
class OLDMANCONFIG_API UGlobalEventName : public UObject
{
	GENERATED_BODY()

public:  
	static const FName Key_Player_OnDeath;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Player_OnDeath() { return Key_Player_OnDeath; }

	static const FName Key_Player_OnRespawn;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Player_OnRespawn() { return Key_Player_OnRespawn; }

	static const FName Key_Player_OnChangeGrivity;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Player_OnChangeGrivity() { return Key_Player_OnChangeGrivity; }

	static const FName Key_Player_ChangeInputActive;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Player_ChangeInputActive() { return Key_Player_ChangeInputActive; }
	
	static const FName Key_Save_SavePointActive;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName") 
	static FName GetKey_Save_SavePointActive() { return Key_Save_SavePointActive; }

	static const FName Key_Input_InputDeviceChanged;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_InputDeviceChanged() { return Key_Input_InputDeviceChanged; }

	static const FName Key_Input_InputPullStart;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_InputPullStart() { return Key_Input_InputPullStart; }

	static const FName Key_Input_InputPullEnd;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_InputPullEnd() { return Key_Input_InputPullEnd; }

	static const FName Key_Input_InputAttack;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_InputAttack() { return Key_Input_InputAttack; }

	static const FName Key_Input_LockMouseKey;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_LockMouseKey() { return Key_Input_LockMouseKey; }

	static const FName Key_Input_UnLockPull;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_UnLockPull() { return Key_Input_UnLockPull; }
	static const FName Key_Input_UnLockAttack;
	UFUNCTION(BlueprintCallable, Category = "GlobalEventName")
	static FName GetKey_Input_UnLockAttack() { return Key_Input_UnLockAttack; }
};
