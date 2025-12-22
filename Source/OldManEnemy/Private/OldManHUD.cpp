// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManHUD.h"
#include "Engine/Canvas.h"

void AOldManHUD::DrawHUD() {
    Super::DrawHUD();
    if (!Canvas) return;
    float ScreenWidth = Canvas->SizeX;
    float ScreenHeight = Canvas->SizeY;
    // 遍历并更新所有墨渍
    for (int32 i = ActiveInks.Num() - 1; i >= 0; --i) {
        FActiveInk& Ink = ActiveInks[i];
        Ink.Age += GetWorld()->GetDeltaSeconds();

        if (Ink.Age >= Ink.Duration) {
            // 移除过期墨渍
            ActiveInks.RemoveAt(i);
            continue;
        }

        // 计算当前透明度（实现淡入淡出效果）
        float Alpha = 1.0f - (Ink.Age / Ink.Duration);
        float ActualWidth = ScreenWidth * Ink.NormalizedWidth;
        float ActualHeight = ScreenHeight * Ink.NormalizedHeight;
        float ActualPosX = (ScreenWidth * Ink.NormalizedPosition.X) - (ActualWidth / 2);
        float ActualPosY = (ScreenHeight * Ink.NormalizedPosition.Y) - (ActualHeight / 2);

        // 使用 DrawTexture 绘制墨渍
        DrawTexture(
            Ink.Texture, 
            Ink.NormalizedPosition.X, 
            Ink.NormalizedPosition.Y, 
            Ink.NormalizedWidth, 
            Ink.NormalizedHeight, 
            0, 0, 1, 1, 
            FLinearColor(1, 1, 1, Alpha), 
            BLEND_Translucent);
    }
}

void AOldManHUD::AddInk(UTexture2D* InkTexture, FVector2D ScreenPosition,float DisplayTime) {
    FActiveInk NewInk;


    // 初始化 NewInk 的各个属性...
    ActiveInks.Add(NewInk);
}