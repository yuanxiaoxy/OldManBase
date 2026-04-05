// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineDeathArea/SplineDeathArea.h"
//#include "Components/CapsuleComponent.h"
//// 【新增】引入原生Debug绘制头文件
//#include "DrawDebugHelpers.h"
//
//ASplineDeathArea::ASplineDeathArea()
//{
//    PrimaryActorTick.bCanEverTick = false; // 彻底关掉Tick，避免麻烦
//    PrimaryActorTick.bTickEvenWhenPaused = false;
//
//    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
//    RootComponent = Spline;
//}
//
//void ASplineDeathArea::OnConstruction(const FTransform& Transform)
//{
//    Super::OnConstruction(Transform);
//    RebuildCollision();
//          
//    // 【关键修改】在这里直接调用绘制，只要变量变化或移动就重画
//    DrawDebugVisualization_Now();
//}
//
//void ASplineDeathArea::BeginPlay()
//{
//    Super::BeginPlay();
//    RebuildCollision();
//    // 游戏开始时可以清空一下屏幕上的调试线
//    FlushPersistentDebugLines(GetWorld());
//}
//
//void ASplineDeathArea::Tick(float DeltaTime)
//{
//    Super::Tick(DeltaTime);
//    // 这个函数现在留空，我们不用它了
//}
//
//void ASplineDeathArea::RebuildCollision()
//{
//    // 1. 清理旧的碰撞体
//    for (auto* Caps : CollisionCapsules)
//    {
//        if (Caps)
//        {
//            Caps->DestroyComponent();
//        }
//    }
//    CollisionCapsules.Reset();
//
//    // 2. 检查样条线有效性
//    const int32 NumNodes = Spline->GetNumberOfSplinePoints();
//    if (NumNodes < 2) return;
//
//    // 3. 同步 NodeRadii 数组长度
//    if (NodeRadii.Num() != NumNodes)
//    {
//        NodeRadii.SetNum(NumNodes);
//        for (float& R : NodeRadii)
//        {
//            if (R == 0) R = Radius;
//        }
//    }
//
//    // 4. 计算分段
//    const float SplineLen = Spline->GetSplineLength();
//    const int32 NumSeg = FMath::Max(1, FMath::RoundToInt(SplineLen * SegmentsPerMeter));
//    const float Step = SplineLen / NumSeg;
//
//    // Lambda：获取距离处的半径
//    auto GetRadiusAtDistance_Local = [&](float D)->float
//        {
//            if (NodeRadii.Num() < 2) return Radius;
//
//            const float Key = Spline->GetInputKeyValueAtDistanceAlongSpline(D);
//            const int32 Idx = FMath::Clamp(FMath::FloorToInt(Key), 0, NodeRadii.Num() - 2);
//            const float Alpha = Key - Idx;
//            return FMath::Lerp(NodeRadii[Idx], NodeRadii[Idx + 1], Alpha);
//        };
//
//    // 5. 生成碰撞体
//    for (int32 i = 0; i < NumSeg; ++i)
//    {
//        const float t1 = i * Step;
//        const float t2 = (i + 1) * Step;
//
//        FVector pos1 = Spline->GetLocationAtDistanceAlongSpline(t1, ESplineCoordinateSpace::Local);
//        FVector pos2 = Spline->GetLocationAtDistanceAlongSpline(t2, ESplineCoordinateSpace::Local);
//        FVector dir = (pos2 - pos1).GetSafeNormal();
//        float segLen = FVector::Dist(pos1, pos2);
//
//        if (segLen <= 0) continue;
//
//        float r1 = GetRadiusAtDistance_Local(t1);
//        float r2 = GetRadiusAtDistance_Local(t2);
//        float avgR = (r1 + r2) * 0.5f;
//
//        UCapsuleComponent* Caps = NewObject<UCapsuleComponent>(this);
//        Caps->RegisterComponent();
//        Caps->AttachToComponent(Spline, FAttachmentTransformRules::KeepRelativeTransform);
//
//        // 强制隐藏
//        Caps->SetVisibility(false);
//        Caps->SetHiddenInGame(true);
//
//        Caps->SetCapsuleRadius(avgR);
//        Caps->SetCapsuleHalfHeight(segLen * 0.5f);
//
//        FVector CenterPos = pos1 + dir * segLen * 0.5f;
//        Caps->SetRelativeLocation(CenterPos);
//        Caps->SetRelativeRotation(dir.ToOrientationRotator());
//
//        Caps->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
//        Caps->SetCollisionResponseToAllChannels(ECR_Ignore);
//        Caps->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
//
//        Caps->OnComponentBeginOverlap.AddDynamic(this, &ASplineDeathArea::OnCapsuleBeginOverlap);
//        Caps->OnComponentEndOverlap.AddDynamic(this, &ASplineDeathArea::OnCapsuleEndOverlap);
//
//        CollisionCapsules.Add(Caps);
//    }
//}
//
//void ASplineDeathArea::OnCapsuleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//    if (APawn* Pawn = Cast<APawn>(OtherActor))
//    {
//        OnPlayerEnterDeathArea.Broadcast(Pawn);
//    }
//}
//
//void ASplineDeathArea::OnCapsuleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
//{
//    if (APawn* Pawn = Cast<APawn>(OtherActor))
//    {
//        OnPlayerExitDeathArea.Broadcast(Pawn);
//    }
//}
//
//#if WITH_EDITOR
//void ASplineDeathArea::PostEditMove(bool bFinished)
//{
//    Super::PostEditMove(bFinished);
//
//    // 移动时也强制重绘
//    if (bLiveUpdate)
//    {
//        RebuildCollision();
//    }
//
//    if (bFinished)
//    {
//        DrawDebugVisualization_Now();
//    }
//}
//
//// 【新增】这个是新的绘制函数，不依赖Tick
//void ASplineDeathArea::DrawDebugVisualization_Now()
//{
//    // 如果开关没开，或者是在游戏世界里，直接返回
//    if (!bShowDebugVisualization || !GetWorld() || !Spline) return;
//    if (GetWorld()->IsGameWorld()) return;
//
//    // 先清除旧的线（针对这个Actor的）
//    // 注意：FlushPersistentDebugLines 会清掉所有，但为了简单我们先这样用
//    FlushPersistentDebugLines(GetWorld());
//
//    const int32 NumNodes = Spline->GetNumberOfSplinePoints();
//    if (NumNodes < 2) return;
//
//    const float TotalLen = Spline->GetSplineLength();
//    const int32 VisualSteps = FMath::Clamp(FMath::CeilToInt(TotalLen / 15.0f), 10, 100);
//
//    // 1. 先画一条贯穿的中心线，确保你能看到路径
//    TArray<FVector> LinePoints;
//    for (int32 i = 0; i <= VisualSteps; ++i)
//    {
//        float Dist = (TotalLen / VisualSteps) * i;
//        FVector Loc = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
//        LinePoints.Add(Loc);
//    }
//    // 绘制线条（保留60秒，红色）
//    DrawDebugLine(GetWorld(), LinePoints[0], LinePoints[LinePoints.Num() - 1], FColor::Red, false, 60.0f, 0, 2.0f);
//    // 或者画多段线：
//    for (int32 i = 0; i < LinePoints.Num() - 1; i++)
//    {
//        DrawDebugLine(GetWorld(), LinePoints[i], LinePoints[i + 1], DebugColor, false, 60.0f, 0, 3.0f);
//    }
//
//    // 2. 再画球体
//    for (int32 i = 0; i <= VisualSteps; ++i)
//    {
//        float Dist = (TotalLen / VisualSteps) * i;
//        FVector Loc = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);
//
//        // 计算半径
//        float R = Radius;
//        if (NodeRadii.Num() >= 2)
//        {
//            const float Key = Spline->GetInputKeyValueAtDistanceAlongSpline(Dist);
//            const int32 Idx = FMath::Clamp(FMath::FloorToInt(Key), 0, NodeRadii.Num() - 2);
//            const float Alpha = Key - Idx;
//            R = FMath::Lerp(NodeRadii[Idx], NodeRadii[Idx + 1], Alpha);
//        }
//
//        // 【关键】使用 DrawDebugSphere 原生版本
//        // 参数说明: World, Center, Radius, Segments, Color, Persistent (false), LifeTime, DepthPriority, Thickness
//        DrawDebugSphere(GetWorld(), Loc, R, 12, DebugColor, false, 60.0f, 0, 1.0f);
//    }
//}
//
//// 保留旧的函数声明但实现为空，防止链接错误
//void ASplineDeathArea::DrawDebugVisualization() const
//{
//    // Do nothing
//}
//#endif