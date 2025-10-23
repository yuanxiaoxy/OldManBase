// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhoneNumberActor/PhoneNumberActor.h"
#include "PhoneNumberActor/PhoneNumberFinishActor.h"
#include "LineGenerate/LineGenerator.h"
#include "PhoneNumberManager.generated.h"

class APhoneNumberActor;
class APhoneNumberFinishActor;
class ULineGenerator;

USTRUCT(BlueprintType)
struct FPhoneNumberData
{
    GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	FString TargetSecret;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PhoneNumber")
	FString CurSecret;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	bool Immediately;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	APhoneNumberFinishActor* PhoneNumberTriggerActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhoneNumber")
	TArray<APhoneNumberActor*> PhoneNumbers;

	// 该组的线段生成器
	ULineGenerator* LineGenerator;

	// 线段生成的参数
	UPROPERTY(EditAnywhere, Category = "Line Generation")
	UStaticMesh* LineStaticMesh;

	UPROPERTY(EditAnywhere, Category = "Line Generation")
	float LineWidth = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Line Generation")
	TEnumAsByte<ESplineMeshAxis::Type> LineForwardAxis = ESplineMeshAxis::Z;

	UPROPERTY(EditAnywhere, Category = "Line Generation")
	UMaterialInstance* LineMaterial;
};

USTRUCT()
struct FGenerateLineData
{
	GENERATED_BODY()

	UPROPERTY()
	int lastIndex;

	UPROPERTY()
	APhoneNumberActor* LastActivatedPhoneNumber;
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

private:
	// 存储上一个激活的电话数字（用于按顺序连接）
	UPROPERTY()
	TMap<FString, FGenerateLineData> LastActivatedPhoneNumber;

public:
	UFUNCTION(BlueprintCallable)
	void InitSecretGroupByGroupName(FString groupName);

	UFUNCTION(BlueprintCallable)
	void ResetSecretGroupByName(FString groupName);

	UFUNCTION(BlueprintCallable)
	void ResetAllSecretGroup();

	UFUNCTION(BlueprintCallable)
	void EnablePhoneNumberByGroupName(FString name, int number, APhoneNumberActor* curActivatedPhoneNumber);
	
	//// 清除所有线段
	//UFUNCTION(BlueprintCallable)
	//void ClearLinesByGroupName();

	//// 清除所有线段
	//UFUNCTION(BlueprintCallable)
	//void ClearAllLines();

private:
	UFUNCTION()
	void InitAllSecretGroup();

	UFUNCTION()
	// 生成线段
	void GenerateLineBetweenActors(FString GroupName, FGenerateLineData& lineData, APhoneNumberActor* ToActor);
};
