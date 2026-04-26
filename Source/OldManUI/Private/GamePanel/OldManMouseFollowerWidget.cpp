// Fill out your copyright notice in the Description page of Project Settings.

#include "GamePanel/OldManMouseFollowerWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "GameFramework/PlayerController.h"

// 静态辅助函数（完全避开 UE 反射系统，避免 Slider 委托冲突）
void UOldManMouseFollowerWidget::EnableFollow(UImage* InImage, const FVector2D& InSize, float& OutHalfWidth, float& OutHalfHeight)
{
    if (!InImage) return;
    InImage->SetBrushSize(InSize);
    OutHalfWidth = InSize.X * 0.5f;
    OutHalfHeight = InSize.Y * 0.5f;
}

void UOldManMouseFollowerWidget::UpdateFollow(UImage* InImage, APlayerController* PC, float InHalfWidth, float InHalfHeight)
{
    if (!InImage || !PC) return;

    UCanvasPanelSlot* tempSlot = Cast<UCanvasPanelSlot>(InImage->Slot);
    if (!Slot) return;

    float MouseX, MouseY;
    if (PC->GetMousePosition(MouseX, MouseY))
    {
        FVector2D NewPosition(MouseX - InHalfWidth, MouseY - InHalfHeight);
        tempSlot->SetPosition(NewPosition);
    }
}

void UOldManMouseFollowerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!FollowerImage)
    {
        UE_LOG(LogTemp, Error, TEXT("MouseFollowerWidget: FollowerImage is not bound! Please name the Image widget 'FollowerImage'."));
        return;
    }

    // 设定图片大小为 100x100 并计算半宽半高
    EnableFollow(FollowerImage, FVector2D(100.0f, 100.0f), HalfWidth, HalfHeight);

    // 可选：显示鼠标光标并设置输入模式
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeGameAndUI());
    }
}

void UOldManMouseFollowerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!FollowerImage) return;

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        UpdateFollow(FollowerImage, PC, HalfWidth, HalfHeight);
    }
}