// OldManCameraAnimationTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "OldManCameraAnimationAsset.generated.h"

UENUM(BlueprintType)
enum class ECameraAnimationType : uint8
{
    LookAtObject UMETA(DisplayName = "看向物体"),
    LookAtDirection UMETA(DisplayName = "看向方向")
};

UENUM(BlueprintType)
enum class ECameraMovementType : uint8
{
    FollowObject UMETA(DisplayName = "跟随物体"),
    FixedPosition UMETA(DisplayName = "固定位置"),
    OrbitObject UMETA(DisplayName = "绕物体旋转"),
    FollowPath UMETA(DisplayName = "按轨迹运动")
};

USTRUCT(BlueprintType)
struct FOldManCameraAnimationData
{
    GENERATED_BODY()

    // 基础设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    ECameraAnimationType AnimationType = ECameraAnimationType::LookAtObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    ECameraMovementType MovementType = ECameraMovementType::FixedPosition;

    // 看向物体设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Look At Object")
    AActor* TargetObject = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Look At Object")
    FVector TargetOffset = FVector::ZeroVector;

    // 看向方向设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Look At Direction")
    FRotator TargetRotation = FRotator::ZeroRotator;

    // 轨道设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Orbit")
    float OrbitRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Orbit")
    float OrbitSpeed = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Orbit")
    float CurrentOrbitAngle = 0.0f;

    // 路径设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Path")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Path")
    float PathSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Path")
    bool bLoopPath = false;

    // 通用设置
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    float BlendInTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    float BlendOutTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    float CameraFOV = 90.0f;

    // 曲线控制
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Curves")
    UCurveFloat* BlendCurve = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation|Curves")
    UCurveFloat* PathCurve = nullptr;

    // 运行时变量
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Animation|Runtime")
    int32 CurrentPathIndex = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Animation|Runtime")
    float PathProgress = 0.0f;
};

UCLASS(BlueprintType)
class OLDMAN_API UOldManCameraAnimationAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    FOldManCameraAnimationData CameraAnimationData;
};