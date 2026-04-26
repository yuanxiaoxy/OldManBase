// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OldManTipLight.h"
#include "OldManBossHead.generated.h"


UCLASS()
class OLDMAN_API AOldManBossHead : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOldManBossHead();
	UFUNCTION(BlueprintCallable)
	void UpdateData();		//根据记录数据更新老人头位置
	UFUNCTION(BlueprintCallable)
	void UpdateInitPodRot();		//根据记录数据更新老人头位置

	UFUNCTION(BlueprintCallable)
	void SetPartActive(ECurOperationType target, bool Active);		//设置指定部位激活part
	UFUNCTION(BlueprintCallable)
	void SetAllPartActive(bool Active);		//设置所有部位激活part
	UFUNCTION(BlueprintCallable)
	void SetAllPartBack();		//恢复所有部位
	//左眉毛
	UFUNCTION(BlueprintCallable)
	void LeftEyebrowOpen();		//张开左眉毛
	UFUNCTION(BlueprintCallable)
	void LeftEyebrowClose();		//关闭左眉毛
	UFUNCTION(BlueprintCallable)
	void LeftEyebrowBlink();		//左眉毛闪烁
	//右眉毛
	UFUNCTION(BlueprintCallable)
	void RightEyebrowOpen();		//张开右眉毛
	UFUNCTION(BlueprintCallable)
	void RightEyebrowClose();		//关闭右眉毛
	UFUNCTION(BlueprintCallable)
	void RightEyebrowBlink();		//右眉毛闪烁
	//下巴
	UFUNCTION(BlueprintCallable)
	void ChinOpen();		//张开下巴
	UFUNCTION(BlueprintCallable)
	void ChinClose();		//关闭下巴
	UFUNCTION(BlueprintCallable)
	void ChinBlink();		//下巴闪烁
	//左耳朵
	UFUNCTION(BlueprintCallable)
	void LeftEarDragedAdd(float Progress);		//左耳朵拉住add方法
	UFUNCTION(BlueprintCallable)
	void LeftEarDraged(float curProgress);		//左耳朵拉住
	UFUNCTION(BlueprintCallable)
	void LeftEarBack();		//左耳朵回正
	UFUNCTION(BlueprintCallable)
	void LeftEarBlink();		//下左耳朵闪烁
	UFUNCTION(BlueprintCallable)
	void JudgeLeftEarRight();		//判断左耳朵是否在正确位置
	//右耳朵
	UFUNCTION(BlueprintCallable)
	void RightEarDragedAdd(float Progress);		//右耳朵拉住add方法
	UFUNCTION(BlueprintCallable)
	void RightEarDraged(float curProgress);		//右耳朵拉住
	UFUNCTION(BlueprintCallable)
	void RightEarBack();		//右耳朵回正
	UFUNCTION(BlueprintCallable)
	void RightEarBlink();		//下右耳朵闪烁
	UFUNCTION(BlueprintCallable)
	void JudgeRightEarRight();		//判断右耳朵是否在正确位置
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	//物体移动速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AllSpeed")
	float AllSpeed = 10;
	//物体移动速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AllSpeed")
	float AllEarSpeed = 1;
	//物体移动速度
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	bool AllPartsMovingEnd = true;

	//左眉毛
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEyebrow")
	bool IsLeftEyebrowActive = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LeftEyebrow")
	bool IsLeftEyebrowOpen = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEyebrow")
	AActor* LeftEyebrow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEyebrow")
	FVector LeftEyebrowOpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEyebrow")
	FRotator LeftEyebrowOpenRot;
	//右眉毛
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEyebrow")
	bool IsRightEyebrowActive = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RightEyebrow")
	bool IsRightEyebrowOpen = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEyebrow")
	AActor* RightEyebrow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEyebrow")
	FVector RightEyebrowOpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEyebrow")
	FRotator RightEyebrowOpenRot;
	//下巴
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	bool IsChinActive = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chin")
	bool IsChinOpen = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	AActor* Chin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FVector ChinOpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FRotator ChinOpenRot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	AActor* ShangBaL;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FVector ShangBaLOpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FRotator ShangBaLOpenRot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	AActor* ShangBaMid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FVector ShangBaMidOpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FRotator ShangBaMidOpenRot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	AActor* ShangBaR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FVector ShangBaROpenPos;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chin")
	FRotator ShangBaROpenRot;
	//左耳朵
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEar")
	bool IsLeftEarActive = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LeftEar")
	bool IsLeftEarRight = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LeftEar", meta = (ClampMin = 0f, ClampMax = 1f))
	float LeftEarProgress = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEar")
	AActor* LeftEar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEar")
	FRotator LeftEarMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LeftEar")
	FRotator LeftEarDetectionRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LeftEar")
	bool LeftEarBackCompelete = false;

	//右耳朵
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEar")
	bool IsRightEarActive = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RightEar")
	bool IsRightEarRight = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RightEar", meta = (ClampMin = 0f, ClampMax = 1f))
	float RightEarProgress = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEar")
	AActor* RightEar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEar")
	FRotator RightEarMax;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RightEar")
	FRotator RightEarDetectionRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RightEar")
	bool RightEarBackCompelete = false;

	//当前阶段
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RightEar")
	ECurOperationType CurType = ECurOperationType::None;

private:
	//总控开关
	bool CanRunning = true;
	//原位置和旋转
	//左眉毛
	FVector LeftEyebrowInitialPos;
	FRotator LeftEyebrowInitialRot;
	bool IsLeftEyebrowMoving = false;
	//右眉毛
	FVector RightEyebrowInitialPos;
	FRotator RightEyebrowInitialRot;
	bool IsRightEyebrowMoving = false;
	//下巴
	FVector ChinInitialPos;
	FRotator ChinInitialRot;
	bool IsChinAllMoving = false;
	bool IsChinMoving = false;
	FVector ShangBaLInitialPos;
	FRotator ShangBaLInitialRot;
	bool IsShangBaLMoving = false;
	FVector ShangBaMidInitialPos;
	FRotator ShangBaMidInitialRot;
	bool IsShangBaMidMoving = false;
	FVector ShangBaRInitialPos;
	FRotator ShangBaRInitialRot;
	bool IsShangBaRMoving = false;
	//左耳朵
	FRotator LeftEarInitialRot;
	bool IsLeftEarBackMoving = false;
	//右耳朵
	FRotator RightEarInitialRot;
	bool IsRightEarBackMoving = false;

};
