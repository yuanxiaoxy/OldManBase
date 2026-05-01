// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UIManager/UIBase.h"
#include "Components/CanvasPanelSlot.h"
#include "OldManMouseFollowerWidget.generated.h"

class UImage;

/**
 *
 */
UCLASS()
class OLDMANUI_API UOldManMouseFollowerWidget : public UUIBase
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
    /** 需要跟随鼠标的图片控件（必须在蓝图中命名为 FollowerImage） */
    UPROPERTY(meta = (BindWidget))
    UImage* FollowerImage;

    /** 图片的半宽（50） */
    float HalfWidth;

    /** 图片的半高（50） */
    float HalfHeight;

private:
    /** 使图片跟随鼠标的核心逻辑（静态函数，避免委托冲突） */
    void EnableFollow(UImage* InImage, const FVector2D& InSize, float& OutHalfWidth, float& OutHalfHeight);
    void UpdateFollow(UImage* InImage, APlayerController* PC, float HalfWidth, float HalfHeight);
};