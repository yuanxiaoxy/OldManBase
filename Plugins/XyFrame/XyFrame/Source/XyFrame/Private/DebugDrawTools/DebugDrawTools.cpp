#include "DebugDrawTools/DebugDrawTools.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#if WITH_EDITOR
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#endif

// 静态成员初始化
bool UDebugDrawTools::bDebugDrawEnabled = true;

UWorld* UDebugDrawTools::GetCurrentWorld()
{
    if (GEngine == nullptr) return nullptr;

    // 优先返回 PIE / Game world
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.World() && (Ctx.WorldType == EWorldType::PIE || Ctx.WorldType == EWorldType::Game))
        {
            return Ctx.World();
        }
    }

    // 再尝试第一个有效 world（编辑器模式）
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.World())
        {
            return Ctx.World();
        }
    }

    return nullptr;
}

float UDebugDrawTools::CalculateDuration(float RequestedDuration, EDebugDrawDuration DrawDuration)
{
    switch (DrawDuration)
    {
    case EDebugDrawDuration::OneFrame:
        return 0.0f;
    case EDebugDrawDuration::Persistent:
        return -1.0f; // persistent marker used by DrawDebug helpers
    case EDebugDrawDuration::CustomTime:
        return RequestedDuration;
    default:
        return RequestedDuration;
    }
}

// 关键执行点：现在接收右值 lambda
void UDebugDrawTools::ExecuteDebugDraw(UWorld* World, float ActualDuration, TFunction<void()>&& DrawFunction)
{
    if (!bDebugDrawEnabled || !World) return;

    // 直接执行传入 lambda
    if (DrawFunction)
    {
        DrawFunction();
    }
}

/** ----------------- Drawing implementations ----------------- **/

void UDebugDrawTools::DrawDebugLine(const FVector& Start, const FVector& End, FColor Color,
    float Duration, float Thickness, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;

    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Start, End, Color, ActualDuration, Thickness]()
        {
            ::DrawDebugLine(World, Start, End, Color, false, ActualDuration, 0, Thickness);
        });
}

void UDebugDrawTools::DrawDebugArrow(const FVector& Start, const FVector& End,
    float ArrowSize, float ArrowThickness, FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Start, End, ArrowSize, ArrowThickness, Color, ActualDuration]()
        {
            ::DrawDebugDirectionalArrow(World, Start, End, ArrowSize, Color, false, ActualDuration, 0, ArrowThickness);
        });
}

void UDebugDrawTools::DrawDebugSphere(const FVector& Center, float Radius, int32 Segments,
    FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Center, Radius, Segments, Color, ActualDuration]()
        {
            ::DrawDebugSphere(World, Center, Radius, Segments, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugBox(const FVector& Center, const FVector& Extent, FColor Color,
    float Duration, const FRotator& Rotation, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Center, Extent, Color, ActualDuration, Rotation]()
        {
            FQuat RotQuat = Rotation.Quaternion();
            ::DrawDebugBox(World, Center, Extent, RotQuat, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugCapsule(const FVector& Center, float HalfHeight, float Radius, const FQuat& Rotation,
    FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Center, HalfHeight, Radius, Rotation, Color, ActualDuration]()
        {
            ::DrawDebugCapsule(World, Center, HalfHeight, Radius, Rotation, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugCylinder(const FVector& Start, const FVector& End, float Radius,
    int32 Segments, FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Start, End, Radius, Segments, Color, ActualDuration]()
        {
            ::DrawDebugCylinder(World, Start, End, Radius, Segments, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugCone(const FVector& Origin, const FVector& Direction,
    float Length, float AngleWidth, float AngleHeight,
    int32 Segments, FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Origin, Direction, Length, AngleWidth, AngleHeight, Segments, Color, ActualDuration]()
        {
            ::DrawDebugCone(World, Origin, Direction, Length, FMath::DegreesToRadians(AngleWidth), FMath::DegreesToRadians(AngleHeight),
                Segments, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugSector(const FVector& Center, const FVector& Direction, float Radius,
    float AngleDeg, int32 Segments, FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    if (Segments <= 0) return;
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    // Build points along the arc and draw lines between them
    FVector Dir = Direction.GetSafeNormal();
    const float Half = AngleDeg * 0.5f;
    const FRotator Rot = Dir.Rotation();
    const float Step = AngleDeg / float(Segments);
    TArray<FVector> Points;
    Points.Reserve(Segments + 2);
    for (int32 i = 0; i <= Segments; ++i)
    {
        float Angle = -Half + Step * i;
        FRotator R = Rot;
        R.Yaw += Angle;
        FVector P = Center + R.Vector() * Radius;
        Points.Add(P);
    }

    for (int32 i = 0; i + 1 < Points.Num(); ++i)
    {
        ExecuteDebugDraw(World, ActualDuration, [World, A = Points[i], B = Points[i + 1], Color, ActualDuration]()
            {
                ::DrawDebugLine(World, A, B, Color, false, ActualDuration, 0, 1.0f);
            });
    }

    // draw radius lines
    ExecuteDebugDraw(World, ActualDuration, [World, Center, P0 = Points[0], Pn = Points.Last(), Color, ActualDuration]()
        {
            ::DrawDebugLine(World, Center, P0, Color, false, ActualDuration, 0, 1.0f);
            ::DrawDebugLine(World, Center, Pn, Color, false, ActualDuration, 0, 1.0f);
        });
}

void UDebugDrawTools::DrawDebugPoint(const FVector& Position, float Size, FColor Color,
    float Duration, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Position, Size, Color, ActualDuration]()
        {
            ::DrawDebugPoint(World, Position, Size, Color, false, ActualDuration, 0);
        });
}

void UDebugDrawTools::DrawDebugString(const FString& Text, const FVector& Location, FColor Color,
    float Duration, bool DrawShadow, float FontScale, EDebugDrawDuration DrawDuration)
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    ExecuteDebugDraw(World, ActualDuration, [World, Text, Location, Color, DrawShadow, FontScale, ActualDuration]()
        {
            ::DrawDebugString(World, Location, Text, nullptr, Color, ActualDuration, DrawShadow, FontScale);
        });
}

void UDebugDrawTools::DrawDebugFloat(const FString& Name, float Value, const FVector& Location,
    FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    DrawDebugString(FString::Printf(TEXT("%s: %.3f"), *Name, Value), Location, Color, Duration, true, 1.0f, DrawDuration);
}

void UDebugDrawTools::DrawDebugVector(const FString& Name, const FVector& Value, const FVector& Location,
    FColor Color, float Duration, EDebugDrawDuration DrawDuration)
{
    DrawDebugString(FString::Printf(TEXT("%s: %s"), *Name, *Value.ToString()), Location, Color, Duration, true, 1.0f, DrawDuration);
}

void UDebugDrawTools::BatchDrawDebugLines(const TArray<FVector>& Points, FColor Color,
    float Duration, float Thickness, EDebugDrawDuration DrawDuration)
{
    if (Points.Num() < 2) return;
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    float ActualDuration = CalculateDuration(Duration, DrawDuration);

    for (int32 i = 0; i + 1 < Points.Num(); i += 2)
    {
        const FVector A = Points[i];
        const FVector B = Points[i + 1];
        ExecuteDebugDraw(World, ActualDuration, [World, A, B, Color, ActualDuration, Thickness]()
            {
                ::DrawDebugLine(World, A, B, Color, false, ActualDuration, 0, Thickness);
            });
    }
}

void UDebugDrawTools::SetDebugDrawEnabled(bool bEnabled)
{
    bDebugDrawEnabled = bEnabled;
}

bool UDebugDrawTools::IsDebugDrawEnabled()
{
    return bDebugDrawEnabled;
}

void UDebugDrawTools::ClearAllDebugShapes()
{
    UWorld* World = GetCurrentWorld();
    if (!World) return;
    FlushPersistentDebugLines(World);
}

void UDebugDrawTools::DrawDebugInEditor(const FVector& Location, const FString& Text, FColor Color)
{
#if WITH_EDITOR
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, Color, Text);
    }
    UWorld* World = GetCurrentWorld();
    if (World)
    {
        ::DrawDebugString(World, Location, Text, nullptr, Color, 5.0f, true, 1.0f);
    }
#endif
}

FColor UDebugDrawTools::GetColorByIndex(int32 Index)
{
    static const TArray<FColor> Colors = {
        FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow,
        FColor::Cyan, FColor::Magenta, FColor::Orange, FColor::Purple,
        FColor::Emerald, FColor::Silver
    };

    if (Colors.Num() == 0) return FColor::White;
    return Colors[FMath::Abs(Index) % Colors.Num()];
}

bool UDebugDrawTools::IsInEditorMode()
{
    UWorld* World = GetCurrentWorld();
    return World && (World->WorldType == EWorldType::Editor);
}
