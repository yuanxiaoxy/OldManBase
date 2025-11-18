// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "DebugDrawTools.generated.h"

/**
 * 调试绘制持续时间类型
 */
UENUM(BlueprintType)
enum class EDebugDrawDuration : uint8
{
    OneFrame     UMETA(DisplayName = "SingleFrame"),
    Persistent   UMETA(DisplayName = "Persistent"),
    CustomTime   UMETA(DisplayName = "CustomTime")
};

/**
 * 调试绘制工具
 */
UCLASS()
class UDebugDrawTools : public USingletonBase
{
    GENERATED_BODY()

public:
    // 你原先的构造/单例如果有额外要求请保留

    /** Lines */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Lines")
    static void DrawDebugLine(const FVector& Start, const FVector& End,
        FColor Color = FColor::White, float Duration = 0.0f,
        float Thickness = 1.0f, EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Lines")
    static void DrawDebugArrow(const FVector& Start, const FVector& End,
        float ArrowSize = 20.f, float ArrowThickness = 1.0f,
        FColor Color = FColor::White, float Duration = 0.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Primitives */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugSphere(const FVector& Center, float Radius = 50.0f,
        int32 Segments = 12, FColor Color = FColor::White,
        float Duration = 0.0f, EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugBox(const FVector& Center, const FVector& Extent,
        FColor Color = FColor::White, float Duration = 0.0f,
        const FRotator& Rotation = FRotator::ZeroRotator,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugCapsule(const FVector& Center, float HalfHeight, float Radius, const FQuat& Rotation,
        FColor Color = FColor::White, float Duration = 0.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugCylinder(const FVector& Start, const FVector& End, float Radius,
        int32 Segments = 12, FColor Color = FColor::White,
        float Duration = 0.0f, EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugCone(const FVector& Origin, const FVector& Direction,
        float Length, float AngleWidth, float AngleHeight,
        int32 Segments, FColor Color, float Duration,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Sector / Fan (custom implementation) */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Shapes")
    static void DrawDebugSector(const FVector& Center, const FVector& Direction, float Radius,
        float AngleDeg, int32 Segments, FColor Color,
        float Duration = 0.0f, EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Point / small helpers */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Points")
    static void DrawDebugPoint(const FVector& Position, float Size = 10.0f,
        FColor Color = FColor::White, float Duration = 0.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Text / strings */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Text")
    static void DrawDebugString(const FString& Text, const FVector& Location,
        FColor Color = FColor::White, float Duration = 0.0f,
        bool DrawShadow = true, float FontScale = 1.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Text")
    static void DrawDebugFloat(const FString& Name, float Value, const FVector& Location,
        FColor Color = FColor::White, float Duration = 0.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Text")
    static void DrawDebugVector(const FString& Name, const FVector& Value, const FVector& Location,
        FColor Color = FColor::White, float Duration = 0.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Batch helpers */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Batch")
    static void BatchDrawDebugLines(const TArray<FVector>& Points, FColor Color = FColor::White,
        float Duration = 0.0f, float Thickness = 1.0f,
        EDebugDrawDuration DrawDuration = EDebugDrawDuration::OneFrame);

    /** Editor specific helper - show text/message in editor viewport */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Editor")
    static void DrawDebugInEditor(const FVector& Location, const FString& Text, FColor Color = FColor::White);

    /** Control */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Control")
    static void SetDebugDrawEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Control")
    static bool IsDebugDrawEnabled();

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Control")
    static void ClearAllDebugShapes();

    /** Utils */
    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Utilities")
    static FColor GetColorByIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "DebugDraw|Utilities")
    static bool IsInEditorMode();

private:
    // 获取当前世界上下文
    static UWorld* GetCurrentWorld();

    // 计算实际持续时间
    static float CalculateDuration(float RequestedDuration, EDebugDrawDuration DrawDuration);

    // 执行实际的调试绘制 —— 修改：接收右值 lambda，避免 const& 接收临时导致的编译器错误
    static void ExecuteDebugDraw(UWorld* World, float ActualDuration, TFunction<void()>&& DrawFunction);

    // 调试绘制是否启用
    static bool bDebugDrawEnabled;
};
