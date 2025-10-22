#pragma once

#include "CoreMinimal.h"
#include "Components/SplineMeshComponent.h"
#include "LineGenerator.generated.h"

// 线段生成器类
UCLASS(Blueprintable)
class OLDMANITEM_API  ULineGenerator : public UObject
{
    GENERATED_BODY()

public:
    ULineGenerator();
    ~ULineGenerator();

    // 初始化线段组件
    void InitializeSplineComponents(AActor* Owner, UStaticMesh* InStaticMesh,
        ESplineMeshAxis::Type InForwardAxis, float InSplineWidth,
        UMaterialInstance* LineMaterial = nullptr);

    // 生成线段
    void GenerateLine(const FVector& StartPos, const FVector& EndPos, int32 LineIndex = 0);

    // 清除所有线段
    void ClearAllLines();

    // 清除指定线段
    void ClearLine(int32 LineIndex);

    // 获取线段组件
    USplineMeshComponent* GetSplineComponent(int32 Index) const;

    // 设置线段材质
    void SetLineMaterial(UMaterialInstance* Material);

    // 获取当前使用的线段数量
    int32 GetUsedLineCount() const;

private:
    TArray<USplineMeshComponent*> SplineComponents;
    UStaticMesh* StaticMesh;
    ESplineMeshAxis::Type ForwardAxis;
    float SplineWidth;

    // 跟踪哪些线段正在被使用
    TSet<int32> UsedLineIndices;
};