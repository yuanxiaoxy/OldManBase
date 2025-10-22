#include "LineGenerate/LineGenerator.h"

ULineGenerator::ULineGenerator()
    : StaticMesh(nullptr)
    , ForwardAxis(ESplineMeshAxis::Z)
    , SplineWidth(0.5f)
{
}

ULineGenerator::~ULineGenerator()
{
    // 注意：SplineComponents 由UE管理，我们不需要手动删除
    SplineComponents.Empty();
    UsedLineIndices.Empty();
}

void ULineGenerator::InitializeSplineComponents(AActor* Owner, UStaticMesh* InStaticMesh,
    ESplineMeshAxis::Type InForwardAxis, float InSplineWidth,
    UMaterialInstance* LineMaterial)
{
    StaticMesh = InStaticMesh;
    ForwardAxis = InForwardAxis;
    SplineWidth = InSplineWidth;

    // 创建10条线段组件（可根据需要调整数量）
    for (int32 i = 0; i < 10; i++)
    {
        FString ComponentName = FString::Printf(TEXT("SplineMeshComponent_%d"), i);
        USplineMeshComponent* SplineMeshComponent = NewObject<USplineMeshComponent>(Owner, *ComponentName);

        if (SplineMeshComponent)
        {
            SplineMeshComponent->SetMobility(EComponentMobility::Movable);
            SplineMeshComponent->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
            SplineMeshComponent->RegisterComponent();

            // 设置线段属性
            SplineMeshComponent->SetStartScale(FVector2D(SplineWidth, SplineWidth));
            SplineMeshComponent->SetEndScale(FVector2D(SplineWidth, SplineWidth));
            SplineMeshComponent->SetStaticMesh(nullptr);
            SplineMeshComponent->SetForwardAxis(ForwardAxis);

            // 设置材质
            if (LineMaterial)
            {
                SplineMeshComponent->SetMaterial(0, LineMaterial);
            }

            Owner->AddInstanceComponent(SplineMeshComponent);
            SplineComponents.Add(SplineMeshComponent);
        }
    }

    UsedLineIndices.Empty();
}

void ULineGenerator::GenerateLine(const FVector& StartPos, const FVector& EndPos, int32 LineIndex)
{
    if (LineIndex < 0 || LineIndex >= SplineComponents.Num() || !SplineComponents[LineIndex])
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid line index or spline component: %d"), LineIndex);
        return;
    }

    USplineMeshComponent* SplineMesh = SplineComponents[LineIndex];
    SplineMesh->SetStaticMesh(StaticMesh);

    // 计算方向向量
    FVector StartTangent = (EndPos - StartPos).GetSafeNormal();
    FVector EndTangent = StartTangent;

    SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

    // 标记该线段为使用中
    UsedLineIndices.Add(LineIndex);
}

void ULineGenerator::ClearAllLines()
{
    for (USplineMeshComponent* SplineMesh : SplineComponents)
    {
        if (SplineMesh)
        {
            SplineMesh->SetStaticMesh(nullptr);
        }
    }
    UsedLineIndices.Empty();
}

void ULineGenerator::ClearLine(int32 LineIndex)
{
    if (LineIndex >= 0 && LineIndex < SplineComponents.Num() && SplineComponents[LineIndex])
    {
        SplineComponents[LineIndex]->SetStaticMesh(nullptr);
        UsedLineIndices.Remove(LineIndex);
    }
}

USplineMeshComponent* ULineGenerator::GetSplineComponent(int32 Index) const
{
    if (Index >= 0 && Index < SplineComponents.Num())
    {
        return SplineComponents[Index];
    }
    return nullptr;
}

void ULineGenerator::SetLineMaterial(UMaterialInstance* Material)
{
    for (USplineMeshComponent* SplineMesh : SplineComponents)
    {
        if (SplineMesh && Material)
        {
            SplineMesh->SetMaterial(0, Material);
        }
    }
}

int32 ULineGenerator::GetUsedLineCount() const
{
    return UsedLineIndices.Num();
}