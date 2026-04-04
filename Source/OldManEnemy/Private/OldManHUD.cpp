// Fill out your copyright notice in the Description page of Project Settings.


#include "OldManHUD.h"
#include "Engine/Canvas.h"


void AOldManHUD::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("✅ HUD BeginPlay: %p, World: %s"),
        this, *GetWorld()->GetName());
}

void AOldManHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Log, TEXT("❌ HUD EndPlay: %p, 原因: %d"),
        this, (int32)EndPlayReason);
    Super::EndPlay(EndPlayReason);
}



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

        float RotationDeg = Ink.BaseRotationDeg;
        if (!FMath::IsNearlyZero(Ink.SwingAngleDeg))
        {
            const float CwDeg = Ink.BaseRotationDeg - Ink.SwingAngleDeg;
            const float CcwDeg = Ink.BaseRotationDeg + Ink.SwingAngleDeg;

            if (Ink.SwingToggleInterval > 0.0f)
            {
                if (Ink.Age < Ink.SwingBackDelay)
                {
                    RotationDeg = CwDeg;
                }
                else
                {
                    const float Elapsed = Ink.Age - Ink.SwingBackDelay;
                    const int32 Phase = FMath::FloorToInt(Elapsed / Ink.SwingToggleInterval) & 1;
                    RotationDeg = (Phase == 0) ? CwDeg : CcwDeg;
                }
            }
            else
            {
                if (Ink.Age < Ink.SwingBackDelay)
                {
                    RotationDeg = CwDeg;
                }
            }
        }

        // 使用 DrawTexture 绘制墨渍（绕中心旋转）
        DrawTexture(
            Ink.Texture,
            ActualPosX,
            ActualPosY,
            ActualWidth,
            ActualHeight,
            0, 0, 1, 1,
            FLinearColor(1, 1, 1, Alpha),
            BLEND_Translucent,
            1.0f,
            false,
            RotationDeg,
            FVector2D(0.5f, 0.5f));
    }
    
   
}

void AOldManHUD::AddInk(FActiveInk NewInk) 
{
    // 验证this指针有效性
    if (!this || !IsValid(this))
    {
        UE_LOG(LogTemp, Fatal, TEXT("AddInk: HUD实例无效! this=%p"), this);
        return;
    }

    // 验证World上下文
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AddInk: 无法获取World"));
        return;
    }
    ActiveInks.Add(NewInk);
}