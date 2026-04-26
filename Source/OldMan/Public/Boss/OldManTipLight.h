// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/SpotLight.h"
#include "Components/SpotLightComponent.h"
#include "OldManTipLight.generated.h"


UENUM(BlueprintType)
enum class ECurOperationType : uint8
{
	LeftEyebrow,	//张开左眼
	RightEyebrow,	//张开右眼
	Chin,			//张嘴
	TurnHeadLeft,	//左转头 左耳朵
	TurnHeadRight,	//右转头 右耳朵
	Eyebrow,		//张开双眼
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
	int CanMoveIndex = 0;			//可以移动老人头提示材质编号

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	UMaterialInstance* OldManMove;		//可以移动老人头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	UMaterialInstance* OldManStop;		//不可移动老人头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	AActor* SmallLightGreen;		//小绿色灯光提示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OldManCanMove")
	AActor* SmallLightRed;			//小红色灯光提示

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	int BiggerLishtIndex = 2;		//大灯位置

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	AActor* BiggerLightGreen;		//大绿色灯光提示
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	AActor* BiggerLightRed;			//大红色灯光提示

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* LeftEyebrowRun;		//移动左眉毛材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* LeftEyebrowStop;		//移动左眉毛材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* RightEyebrowRun;	//移动右眉毛材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* RightEyebrowStop;	//移动右眉毛材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* EyebrowRun;		//张开双眼材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* EyebrowStop;	//张开双眼材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* ChinRun;			//张嘴材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* ChinStop;			//张嘴材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* TurnHeadLeftRun;	//左转头材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* TurnHeadLeftStop;	//左转头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* TurnHeadRightRun;	//右转头材质
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* TurnHeadRightStop;	//右转头材质

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "OperationMat")
	UMaterialInstance* Wait;			//待机材质

	UFUNCTION(BlueprintCallable)
	void SwitchCanMoveMat(bool CanMove);		//切换提示子物体材质 T：老人头可以移动  F：老人头不可以移动
	
	UFUNCTION(BlueprintCallable)
	void SwitchOperationMat(ECurOperationType TargetOperation);		//切换操作材质
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MyMeshComponent;		//自身组件
private:
	bool CanRunning = true;		//是否可以正常运行
	
	ECurOperationType CurType = ECurOperationType::None;	//当前大灯阶段
	bool OldManHeadCanMove = true;		//当前是否可以移动老人头
	UPointLightComponent* SmallLightGreenPointLight;
	UPointLightComponent* SmallLightRedPointLight;
	UPointLightComponent* BiggerLightGreenPointLight;
	UPointLightComponent* BiggerLightRedPointLight;
	//USceneComponent* RootSceneComponen;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
