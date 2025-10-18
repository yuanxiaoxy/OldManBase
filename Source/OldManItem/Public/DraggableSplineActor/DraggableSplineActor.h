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

    UPROPERTY(BlueprintReadOnly, Category = "Drag")
    bool bIsBeingDragged;

    // ƽ���ƶ���ֵ
    FVector TargetLocation;
    FRotator TargetRotation;
    float MovementAlpha;

    // ���Կ��ӻ�
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugVisualization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugLineLength = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugArrowSize = 50.0f;

    // �������༭��Ԥ������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview",
        meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
    float EditorPreviewPosition = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Editor Preview")
    bool bEnableEditorPreview = false;

    // �༭��Ԥ����ʱ��
    float PreviewTimer;

    // �洢��ʼλ�ú���ת���������ã�
    FVector InitialMeshLocation;
    FRotator InitialMeshRotation;

public:
    UFUNCTION(BlueprintCallable)
    virtual void StartDragging();

    UFUNCTION(BlueprintCallable)
    virtual void StopDragging();

    UFUNCTION(BlueprintCallable)
    virtual void HandleMouseData(const FVector& ViewDirection, float Intensity) override;

    UFUNCTION(BlueprintCallable)
    virtual void MoveBasedOnViewDirection(const FVector& ViewDirection, float Intensity);

    UFUNCTION(BlueprintCallable)
    USplineComponent* GetSplineComponent() const { return SplineComponent; }

    UFUNCTION(BlueprintCallable)
    FVector GetCurrentTangent() const;

    // ���Կ��ӻ�����
    UFUNCTION(BlueprintCallable)
    void SetDebugVisualization(bool bEnable) { bShowDebugVisualization = bEnable; }

    UFUNCTION(BlueprintCallable)
    void ToggleDebugVisualization() { bShowDebugVisualization = !bShowDebugVisualization; }

    // �������༭��Ԥ������
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void UpdateEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ToggleEditorPreview();

    UFUNCTION(CallInEditor, BlueprintCallable, Category = "Editor Preview")
    void ResetEditorPreview();

protected:
    // ���Ƶ��Կ��ӻ�
    void DrawDebugVisualization(const FVector& ViewDirection, float ProjectedMovement);

    // �������༭��Ԥ������
    void UpdatePreviewPosition();

    // ����������������λ�ú���ת
    void SetMeshPositionAndRotation(const FVector& Location, const FRotator& Rotation);
};