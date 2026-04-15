// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OldManTipLight.generated.h"

UENUM(BlueprintType)
enum class ECurOperation : uint8
{
	LeftEyebrow,	//张开左眼
	RightEyebrow,	//张开右眼
	Chin,			//张嘴
	TurnHeadLeft,	//左转头
	TurnHeadRight,	//右转头
	None			//无操作
};

UCLASS()
class OLDMAN_API AOldManTipLight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOldManTipLight();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	AActor* OldManTips;			//可以移动老人头提示物体

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	UMaterial* OldManMove;		//可以移动老人头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	UMaterial* OldManStop;		//不可移动老人头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* LeftEyebrow;		//移动左眉毛材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* RightEyebrow;	//移动右眉毛材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* Chin;			//张嘴材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* TurnHeadLeft;	//左转头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* TurnHeadRight;	//右转头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterial* Wait;			//待机材质

	UFUNCTION(BlueprintCallable)
	void SwitchCanMoveMat(bool CanMove);		//切换提示子物体材质 T：老人头可以移动  F：老人头不可以移动
	
	UFUNCTION(BlueprintCallable)
	void SwitchOperationMat(ECurOperation TargetOperation);		//切换操作材质
private:
	bool CanRunning = true;		//是否可以正常运行
	UStaticMeshComponent* MyMeshComponent;		//自身组件
	UStaticMeshComponent* TipsMeshComponent;		//子对象组件

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
