// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhoneNumberActor/PhoneNumberActor.h"
#include "PhoneNumberActor/PhoneNumberFinishActor.h"
#include "PhoneNumberManager.generated.h"

class APhoneNumberActor;
class APhoneNumberFinishActor;

USTRUCT(BlueprintType)
struct FPhoneNumberData
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	FString TargetSecret;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	FString CurSecret;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	APhoneNumberFinishActor* PhoneNumberTriggerActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	TArray<APhoneNumberActor*> PhoneNumbers;

};

UCLASS(Blueprintable)
class OLDMANITEM_API APhoneNumberManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APhoneNumberManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumberGroup")
	TMap<FString, FPhoneNumberData> PhoneNumberGroup;

public:
	UFUNCTION(BlueprintCallable)
	void InitSecretGroupByGroupName();

	UFUNCTION(BlueprintCallable)
	void ResetAllSecretGroup();

private:
	UFUNCTION()
	void InitAllSecretGroup();
};
