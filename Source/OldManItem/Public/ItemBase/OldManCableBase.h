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
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class USplineComponent* CableSplineComponent;

    UPROPERTY()
    TArray<class USplineMeshComponent*> SplineMeshComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float CableRadius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable")
    float EndDetectionDistance = 100.0f;

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

    // bReverseMovementDirection 已完全删除

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Collision")
    FName CollisionProfileName = TEXT("BlockAll");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cable|Collision")
    bool bGenerateOverlapEvents = false;

public:
    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetStartLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetEndLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    float GetCableLength() const;

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

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector GetTangentAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable")
    FVector MoveAlongCable(const FVector& CurrentPosition, float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void GenerateCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void ClearCableMesh();

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    void UpdateCableVisualization();

    UFUNCTION(BlueprintCallable, Category = "Cable|Collision")
    void UpdateCableCollision();

    UFUNCTION(BlueprintCallable, Category = "Cable|Collision")
    void SetCollisionEnabled(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    FVector GetForwardAxisVector() const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Visual")
    FVector GetTangentAtDistance(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FVector GetUpVectorAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FVector GetRightVectorAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    float CalculateCableRadius() const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FVector GetCharacterPositionOnCable(const FVector& WorldPosition, float CharacterRadius = 50.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FTransform GetTransformAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FRotator GetRotationAtPosition(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    float FindNearestDistanceAlongSpline(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Cable|Navigation")
    FVector GetPositionAtDistance(float Distance) const;

private:
    FVector GetUpVectorAtDistance(float Distance) const;
    FVector GetRightVectorAtDistance(float Distance) const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};