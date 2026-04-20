// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SpotLightComponent.h"
#include "OldManMobileCamera.generated.h"


UCLASS()
class OLDMAN_API AOldManMobileCamera : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AOldManMobileCamera();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void BeginScan(float time);	//开始扫描 有时间
	UFUNCTION(BlueprintCallable)
	void BeginScanWithoutParm();	//开始扫描 无时间
	UFUNCTION(BlueprintCallable)
	void CloseLight();		//关闭灯光
	UFUNCTION(BlueprintImplementableEvent)
	void Open();		//关闭开启
	UFUNCTION(BlueprintImplementableEvent)
	void Close();		//关闭
};
