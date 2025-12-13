// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityActor.h"
#include "FEnemyLocationInfo.h"
#include "EnemyPathDebugger.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANENEMY_API AEnemyPathDebugger : public AEditorUtilityActor
{
	GENERATED_BODY()
public:
		// 每帧调用的绘制函数
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawEnemyDebugInfo(TArray<FEnemyLocationInfo> infos, float lastTime = -1.0f);

	virtual bool ShouldTickIfViewportsOnly() const override;
};
