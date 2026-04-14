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

// ========== 新增委托声明 ==========
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDraggingStarted, ADraggableSplineActor*, DraggedActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDraggingStopped, ADraggableSplineActor*, DraggedActor);
// =================================

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

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    float DragStartPos = 0.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    float DragSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    float MaxDragSpeed = 0.1f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    float DeadZone = 0.05f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    bool SingleDirDrag = true;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    float SmoothingFactor = 0.8f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    bool IfAdjustRotation = true;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag")
    bool IfHasAutoBack = true;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Drag", meta = (ClampMin = 0.1f, ClampMax = 5.0f))
    float AutoBackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Debug")
    bool bShowDebugVisualization = true;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Debug")
    float DebugLineLength = 200.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Debug")
    float DebugArrowSize = 50.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Editor Preview",
        meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float EditorPreviewPosition = 0.0f;

    UPROPERTY(EditAnywhere, BluePrintReadWrite, Category = "Editor Preview")
    bool bEnableEditorPreview = false;

    FVector InitialMeshLocation;
    FRotator InitialMeshRotation;

    FVector TargetLocation;
    FRotator TargetRotation;
    float MovementAlpha;

    FVector SmoothedMovementDirection;

private:
    bool inAutoBack;

public:
    virtual void StartDragging() override;
    virtual void StopDragging() override;
    virtual void HandleMouseData(const FVector& ViewDirection, float Intensity) override;

    UFUNCTION(BlueprintCallable)
    USplineComponent* GetSplineComponent() const { return SplineComponent; }

    UFUNCTION(BlueprintCallable)
    FVector GetCurrentTangent() const;

    UFUNCTION(BlueprintCallable)
    void SetStartPosition();

    UFUNCTION(BlueprintCallable)
    void SetDebugVisualization(bool bEnable) { bShowDebugVisualization = bEnable; }

    UFUNCTION(BlueprintCallable)
    void ToggleDebugVisualization() { bShowDebugVisualization = !bShowDebugVisualization; }

    // ========== 新增公共 Getter ==========
    UFUNCTION(BlueprintPure, Category = "Drag")
    float GetCurrentSplinePosition() const { return CurrentSplinePosition; }

    UFUNCTION(BlueprintPure, Category = "Drag")
    float GetDragStartPos() const { return DragStartPos; }
    // ===================================

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void UpdateEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ToggleEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ResetEditorPreview();

    // ========== 新增拖动事件（蓝图可绑定） ==========
    UPROPERTY(BlueprintAssignable, Category = "Drag")
    FOnDraggingStarted OnDraggingStarted;

    UPROPERTY(BlueprintAssignable, Category = "Drag")
    FOnDraggingStopped OnDraggingStopped;
    // =============================================

protected:
    void StartAutoBack();
    void StopAutoBack();

    void DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement);
    void UpdatePreviewPosition();
    void SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation);
    float CalculateNormalizedMovement(const FVector& ViewDirection);
};