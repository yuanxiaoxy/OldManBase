// DraggableSplineActor.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "ItemBase/OldManPullItemBase.h"
#include "DraggableSplineActor.generated.h"

UCLASS()
class OLDMANITEM_API ADraggableSplineActor : public AOldManPullItemBase
{
    GENERATED_BODY()

public:
    ADraggableSplineActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
    USplineComponent* SplineComponent;

    UPROPERTY(BlueprintReadWrite, Category = "Drag")
    float CurrentSplinePosition;

    // 新增：拖动控制参数
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float DragSensitivity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float MaxDragSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float DeadZone = 0.1f;

    // 调试可视化
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugVisualization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLineLength = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugArrowSize = 50.0f;

    // 编辑器预览功能
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview",
        meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float EditorPreviewPosition = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bEnableEditorPreview = false;

    // 存储初始位置和旋转（用于重置）
    FVector InitialMeshLocation;
    FRotator InitialMeshRotation;

    // 新增：平滑移动插值
    FVector TargetLocation;
    FRotator TargetRotation;
    float MovementAlpha;

public:
    virtual void StartDragging() override;

    virtual void StopDragging() override;

    virtual void HandleMouseData(const FVector& ViewDirection, float Intensity) override;

    UFUNCTION(BlueprintCallable)
    virtual void MoveBasedOnViewDirection(const FVector& ViewDirection, float Intensity);

    UFUNCTION(BlueprintCallable)
    USplineComponent* GetSplineComponent() const { return SplineComponent; }

    UFUNCTION(BlueprintCallable)
    FVector GetCurrentTangent() const;

    // 调试可视化函数
    UFUNCTION(BlueprintCallable)
    void SetDebugVisualization(bool bEnable) { bShowDebugVisualization = bEnable; }

    UFUNCTION(BlueprintCallable)
    void ToggleDebugVisualization() { bShowDebugVisualization = !bShowDebugVisualization; }

    // 编辑器预览函数
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void UpdateEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ToggleEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ResetEditorPreview();

protected:
    // 绘制调试可视化
    void DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement);

    // 编辑器预览函数
    void UpdatePreviewPosition();

    // 设置网格体位置和旋转
    void SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation);

    // 新增：计算归一化移动量
    float CalculateNormalizedMovement(const FVector& ViewDirection, float Intensity);
};