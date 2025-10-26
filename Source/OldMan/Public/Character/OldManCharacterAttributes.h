#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "OldManCharacterAttributes.generated.h"

USTRUCT(BlueprintType)
struct FOldManCameraData
{
    GENERATED_BODY()

    // ========== 属性 ==========
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
};

USTRUCT(BlueprintType)
struct FOldManDetectionData
{
    GENERATED_BODY()

    //检测属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float ConeLength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection Settings")
    float ConeAngle = 80.0f;

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

UCLASS(BlueprintType, Blueprintable)
class OLDMAN_API UOldManCharacterAttributes : public UDataAsset
{
	GENERATED_BODY()

public:
    // 移动属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInWalk = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInJump = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInDoubleJump = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MoveSpeedInAir = 600.0f;

    //暂时没有跑步
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    //float RunSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float DoubleJumpVelocity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SpeedChangeRate = 10.0f;
   
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SpeedChangeRateInAir = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float AirControl = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LandDuration = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RotationRate = 10.0f;

    // 相机属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FOldManCameraData OldManCameraData;

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

    // 攻击属性
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 1.0f;
};
