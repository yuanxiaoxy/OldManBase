// LineGenerator.cpp
#include "LineGenerate/LineGenerator.h"

ULineGenerator::ULineGenerator()
    : StaticMesh(nullptr)
    , ForwardAxis(ESplineMeshAxis::Z)
    , SplineWidth(0.5f)
{
}

ULineGenerator::~ULineGenerator()
{
    // 不需要手动清空，弱指针会自动处理
}

void ULineGenerator::InitializeSplineComponents(AActor* Owner, UStaticMesh* InStaticMesh,
    ESplineMeshAxis::Type InForwardAxis, float InSplineWidth,
    UMaterialInstance* LineMaterial)
{
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("ULineGenerator::InitializeSplineComponents: Invalid Owner"));
        return;
    }

    ComponentOwner = Owner;
    StaticMesh = InStaticMesh;
    ForwardAxis = InForwardAxis;
    SplineWidth = InSplineWidth;

    // 清空现有组件
    SplineComponents.Empty();
    UsedLineIndices.Empty();

    // 创建10条线段组件
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

            // 使用弱指针存储
            SplineComponents.Add(SplineMeshComponent);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create SplineMeshComponent %d"), i);
            // 添加空指针以保持索引一致
            SplineComponents.Add(nullptr);
        }
    }
}

void ULineGenerator::GenerateLine(const FVector& StartPos, const FVector& EndPos, int32 LineIndex)
{
    // 验证组件有效性
    if (!ValidateComponents())
    {
        UE_LOG(LogTemp, Warning, TEXT("ULineGenerator::GenerateLine: Components are invalid"));
        return;
    }

    if (LineIndex < 0 || LineIndex >= SplineComponents.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid line index: %d"), LineIndex);
        return;
    }

    TWeakObjectPtr<USplineMeshComponent> SplineMeshPtr = SplineComponents[LineIndex];
    if (!SplineMeshPtr.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Spline component at index %d is invalid"), LineIndex);
        return;
    }

    USplineMeshComponent* SplineMesh = SplineMeshPtr.Get();
    SplineMesh->SetStaticMesh(StaticMesh);

    // 计算方向向量
    FVector StartTangent = (EndPos - StartPos).GetSafeNormal();
    FVector EndTangent = StartTangent;

    SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

    // 标记该线段为使用中
    UsedLineIndices.Add(LineIndex);
}

int32 ULineGenerator::GenerateLineAutoIndex(const FVector& StartPos, const FVector& EndPos)
{
    int32 AvailableIndex = GetAvailableLineIndex();
    if (AvailableIndex != INDEX_NONE)
    {
        GenerateLine(StartPos, EndPos, AvailableIndex);
    }
    return AvailableIndex;
}

void ULineGenerator::ClearAllLines()
{
    for (TWeakObjectPtr<USplineMeshComponent> SplineMeshPtr : SplineComponents)
    {
        if (SplineMeshPtr.IsValid())
        {
            SplineMeshPtr->SetStaticMesh(nullptr);
        }
    }
    UsedLineIndices.Empty();
}

void ULineGenerator::ClearLine(int32 LineIndex)
{
    if (LineIndex >= 0 && LineIndex < SplineComponents.Num())
    {
        TWeakObjectPtr<USplineMeshComponent> SplineMeshPtr = SplineComponents[LineIndex];
        if (SplineMeshPtr.IsValid())
        {
            SplineMeshPtr->SetStaticMesh(nullptr);
        }
        UsedLineIndices.Remove(LineIndex);
    }
}

USplineMeshComponent* ULineGenerator::GetSplineComponent(int32 Index) const
{
    if (Index >= 0 && Index < SplineComponents.Num() && SplineComponents[Index].IsValid())
    {
        return SplineComponents[Index].Get();
    }
    return nullptr;
}

void ULineGenerator::SetLineMaterial(UMaterialInstance* Material)
{
    for (TWeakObjectPtr<USplineMeshComponent> SplineMeshPtr : SplineComponents)
    {
        if (SplineMeshPtr.IsValid() && Material)
        {
            SplineMeshPtr->SetMaterial(0, Material);
        }
    }
}

int32 ULineGenerator::GetUsedLineCount() const
{
    return UsedLineIndices.Num();
}

int32 ULineGenerator::GetAvailableLineIndex() const
{
    for (int32 i = 0; i < SplineComponents.Num(); i++)
    {
        if (!UsedLineIndices.Contains(i))
        {
            return i;
        }
    }
    return INDEX_NONE;
}

bool ULineGenerator::ValidateComponents() const
{
    if (!ComponentOwner.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Component owner is invalid"));
        return false;
    }

    for (int32 i = 0; i < SplineComponents.Num(); i++)
    {
        if (!SplineComponents[i].IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("Spline component %d is invalid"), i);
            return false;
        }
    }

    return true;
}