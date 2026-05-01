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
    // Cable Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USplineComponent* CableSplineComponent;

    UPROPERTY()
    TArray<class USplineMeshComponent*> SplineMeshComponents;

    // Cable Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float CableRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float EndDetectionDistance = 100.0f;

    // Visual Properties
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Visual")
    float TangentScale = 1.0f;

    // Movement Direction (单向滑索专用：true=反向移动，false=正向移动)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Movement")
    bool bReverseMovementDirection = false;

    // Collision Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Collision")
    FName CollisionProfileName = TEXT("BlockAll");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Collision")
    bool bGenerateOverlapEvents = false;

public:
    // Cable Navigation API (virtual)
    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector GetStartLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector GetEndLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual float GetCableLength() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector FindNearestPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector ProjectPositionToCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector ClampPositionToCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual bool IsAtEndOfCable(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector GetDirectionAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector GetTangentAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual FVector MoveAlongCable(const FVector& CurrentPosition, float Distance) const;

    // Cable Visualization
    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    virtual void GenerateCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    virtual void ClearCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    virtual void UpdateCableVisualization();

    // Cable Collision
    UFUNCTION(BlueprintCallable, Category = "Cable|Collision")
    virtual void UpdateCableCollision();

    UFUNCTION(BlueprintCallable, Category = "Cable|Collision")
    virtual void SetCollisionEnabled(bool bEnable);

    // Helper Functions
    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    virtual FVector GetForwardAxisVector() const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    virtual FVector GetTangentAtDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FVector GetUpVectorAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FVector GetRightVectorAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual float CalculateCableRadius() const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FVector GetCharacterPositionOnCable(const FVector& WorldPosition, float CharacterRadius = 50.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FTransform GetTransformAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FRotator GetRotationAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual float FindNearestDistanceAlongSpline(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    virtual FVector GetPositionAtDistance(float Distance) const;

    // 是否为双向滑索（子类可重写）
    UFUNCTION(BlueprintCallable, Category = "Cable")
    virtual bool IsBidirectional() const { return false; }

    // 蓝图事件
    UFUNCTION(BlueprintImplementableEvent, Category = "Cable|Events")
    void CharacterEnterCable(bool CableMoveForward);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cable|Events")
    void CharacterExitCable(bool CableMoveForward);

protected:
    virtual FVector GetUpVectorAtDistance(float Distance) const;
    virtual FVector GetRightVectorAtDistance(float Distance) const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};