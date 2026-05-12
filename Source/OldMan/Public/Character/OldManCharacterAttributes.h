#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "Animation/AnimMontage.h"
#include "OldManCharacterAttributes.generated.h"

class UOldManStateBase;

UENUM(BlueprintType)
enum class EPlayerBaseMoveState : uint8
{
    Idle = 0 UMETA(DisplayName = "Idle"),
    Walk = 1 UMETA(DisplayName = "Walk"),
    Jump = 2 UMETA(DisplayName = "Jump"),
    DoubleJump = 3 UMETA(DisplayName = "DoubleJump"),
    Fall = 4 UMETA(DisplayName = "Fall"),
    Land = 5 UMETA(DisplayName = "Land"),
    LeftHorizontalJump = 6 UMETA(DisplayName = "LeftHorizontalJump"),
    RightHorizontalJump = 7 UMETA(DisplayName = "RightHorizontalJump"),
};

UENUM(BlueprintType)
enum class EPlayerActionState : uint8
{
    Common = 0 UMETA(DisplayName = "Common"),
    OnSlope = 1 UMETA(DisplayName = "OnSlope"),
    OnCable = 2 UMETA(DisplayName = "OnCable"),
    Dead = 3 UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FOldManCameraData
{
    GENERATED_BODY()

    // ========== 属性 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraLagSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraRotationLagSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraPitchMin = -70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraPitchMax = 70.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    bool bUseCameraSmoothing = true;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float CameraRotationInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float InputSmoothingInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Camera")
    float GravityRotationInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraFrameRate = 120.0f;
};

USTRUCT(BlueprintType)
struct FOldManDetectionData
{
    GENERATED_BODY()

    //检测属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float ConeLength = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float ConeAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float AnimReadyTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float AnimTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float CoolDown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    bool DebugMode = false;

    // 在头文件中添加
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    TArray<TEnumAsByte<ECollisionChannel>> DetectionChannels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    FColor DebugColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float DebugDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FOldManCameraHitchcockData
{
    GENERATED_BODY()

    // ========== 属性 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Hitchcock")
    UCurveFloat* FadeInHitchcockCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Hitchcock")
    UCurveFloat* FadeOutHitchcockCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Hitchcock")
    float HitchcockZoomTargetFOV;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Hitchcock")
    float HitchcockZoomTargetDistance;
};

UCLASS(BlueprintType, Blueprintable)
class OLDMAN_API UOldManCharacterAttributes : public UDataAsset
{
	GENERATED_BODY()

public:
    //玩家属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Param")
    float PlayerHealth = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Param")
    float PlayerRespawnTime = 2.0f;

    // 重力属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = 0.5f, ClampMax = 5.0f))
    float GravityInCommonMove = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = 0.5f, ClampMax = 5.0f))
    float GravityInSlope = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity", meta = (ClampMin = 0.5f, ClampMax = 5.0f))
    float GravityInCable = 1.0f;

    // 移动属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInWalk = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInJump = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInDoubleJump = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInAir = 500.0f;

    //暂时没有跑步
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    //float RunSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpForce = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DoubleJumpForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement", meta = (ClampMin = 1.0f, ClampMax = 2.0f))
    float SpeedChangeRate = 2.0f;
   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SpeedChangeRateInAir = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControl = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LandDuration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationRate = 10.0f;

    //滑坡属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float MoveSpeedInSlope = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope", meta = (ClampMin = 1.0f, ClampMax = 2.0f))
    float MoveSpeedMultiInSlope = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope", meta = (ClampMin = 0.0f, ClampMax = 100.0f))
    float RotatorSpeedMultiInSlope = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float MoveSpeedInJumpInSlope = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float MoveSpeedInDoubleJumpInSlope = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float MoveSpeedInAirInSlope = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float FadeInSlopeStateTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float FadeOutSlopeStateTime = 0.1f;

    UPROPERTY(Editanywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float DetectRayLength = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope")
    float JumpForceInSlope = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|slope")
    float DoubleJumpForceInSlope = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|slope")
    float CheckFrequency = 0.1f;

    //缆绳属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float MoveSpeedInCable = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float MoveSpeedOnJumpInCable = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float MoveSpeedOnDoubleJumpInCable = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float MoveSpeedOnAirInCable = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable", meta = (ClampMin = 0.1f, ClampMax = 2.0f))
    float MoveSpeedMutiInEndCable = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float HorizontalJumpDistance = 400.0f; // 水平检测距离

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float HorizontalJumpLength = 100.0f; // 水平检测宽度

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float HorizontalJumpHeight = 200.0f; // 水平检测高度

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float HorizontalJumpForwardOffset = 100.0f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float HorizontalJumpSpeed = 600.0f; 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float ExitWaitTime = 1.0f;

    //暂时没有跑步
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    //float RunSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float JumpForceInCable = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Cable")
    float DoubleJumpForceInCable = 500.0f;

    // 相机属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FOldManCameraData OldManCameraData;

    // 相机属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Hitchcock")
    FOldManCameraHitchcockData OldManCameraHitchcockData;

    // 范围检测属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    FOldManDetectionData OldManDetectionData;

    // 鼠标灵敏度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    float MouseSensitivity = 1.0f;

    // 动画混合属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementBlendInterpSpeed = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float RotationBlendInterpSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DragMontage;
};
