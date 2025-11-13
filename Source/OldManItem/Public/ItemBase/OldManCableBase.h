#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineMeshComponent.h"
#include "OldManCableBase.generated.h"

UENUM(BlueprintType)
enum class ECableForwardAxis : uint8
{
    X UMETA(DisplayName = "X Axis"),
    Y UMETA(DisplayName = "Y Axis"),
    Z UMETA(DisplayName = "Z Axis"),
    NegativeX UMETA(DisplayName = "-X Axis"),
    NegativeY UMETA(DisplayName = "-Y Axis"),
    NegativeZ UMETA(DisplayName = "-Z Axis")
};

UCLASS()
class OLDMANITEM_API AOldManCableBase : public AActor
{
    GENERATED_BODY()

public:
    AOldManCableBase();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    virtual void Tick(float DeltaTime) override;

    // 滑索组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USplineComponent* CableSplineComponent;

    // 样条网格体组件数组（用于可视化）
    UPROPERTY()
    TArray<class USplineMeshComponent*> SplineMeshComponents;

    // 滑索参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float CableRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float EndDetectionDistance = 100.0f;

    // 滑索模型设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    class UStaticMesh* CableMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    class UMaterialInterface* CableMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    ECableForwardAxis CableForwardAxis = ECableForwardAxis::X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    FVector2D CableScale = FVector2D(1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    float SegmentLength = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    bool bEnableCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    bool bShowInEditor = true;

    // 切线缩放因子，控制曲线的平滑度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    float TangentScale = 1.0f;

public:
    // 获取滑索信息
    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetStartLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetEndLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    float GetCableLength() const;

    // 位置相关方法
    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector FindNearestPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector ProjectPositionToCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector ClampPositionToCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    bool IsAtEndOfCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetDirectionAtPosition(const FVector& WorldPosition) const;

    // 沿着滑索移动
    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector MoveAlongCable(const FVector& CurrentPosition, float Distance) const;

    // 模型生成方法
    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void GenerateCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void ClearCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void UpdateCableVisualization();

    // 获取前向轴向量
    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    FVector GetForwardAxisVector() const;

private:
    // 内部辅助方法
    float FindNearestDistanceAlongSpline(const FVector& WorldPosition) const;
    FVector GetPositionAtDistance(float Distance) const;
    FVector GetTangentAtDistance(float Distance) const;

    // 获取样条线在指定距离处的上方向向量
    FVector GetUpVectorAtDistance(float Distance) const;

    // 调试绘制
    void DrawDebugCable();

    // 编辑器相关
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};