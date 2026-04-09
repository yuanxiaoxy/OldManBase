// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OldManUIBase.h"
#include "OldManPopUpUIBase.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANUI_API UOldManPopUpUIBase : public UOldManUIBase
{
	GENERATED_BODY()
	
public:
    virtual void InternalShowUI(UObject* Data = nullptr) override;

    UPROPERTY(BlueprintReadWrite, Category = "Animation", Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* PopupAnimation_in;
    UPROPERTY(BlueprintReadWrite, Category = "Animation", Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* PopupAnimation_out;

    UFUNCTION(BlueprintCallable)
    void FinishPlayerAction();

    UFUNCTION()
    void OnPopupAnimationFinished();
};
