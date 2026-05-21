// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "PasuePanelButtonBase.generated.h"

/**
 * 
 */
UCLASS()
class OLDMANUI_API UPasuePanelButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    void OnLocalSelected();
    UFUNCTION(BlueprintImplementableEvent, Category = "Selected")
    void BP_OnLocalSelected();

    UFUNCTION(BlueprintCallable)
    void OnLocalDisSelected();
    UFUNCTION(BlueprintImplementableEvent, Category = "DisSelected")
    void BP_OnLocalDisSelected();

    UFUNCTION(BlueprintCallable)
    void OnLocalClick();
    UFUNCTION(BlueprintImplementableEvent, Category = "DisSelected")
    void BP_OnLocalClick();

    UFUNCTION(BlueprintCallable)
    void OnLocalIn();
    UFUNCTION(BlueprintImplementableEvent, Category = "DisSelected")
    void BP_OnLocalIn();

    UPROPERTY(BlueprintReadWrite, Category = "Animation", Transient, meta = (BindWidgetAnim, Optional = true))
    UWidgetAnimation* OnHover;
    UPROPERTY(BlueprintReadWrite, Category = "Animation", Transient, meta = (BindWidgetAnim, Optional = true))
    UWidgetAnimation* Selected;
};
