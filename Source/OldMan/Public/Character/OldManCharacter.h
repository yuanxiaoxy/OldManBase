#pragma once

#include "CoreMinimal.h"
#include "XyCharacter/XyCharacterBase.h"
#include "OldManCharacterAttributes.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/OldManCameraComponent.h"
#include "Character/OldManMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ItemBase/OldManInterectItemBase.h"
#include "ItemBase/OldManPullItemBase.h"
#include "ItemBase/OldManCanBeAttackItemBase.h"
#include "ItemBase/OldManBulletBase.h"
#include "CameraAnimation/OldManCameraAnimationAsset.h"
#include "CameraAnimation/OldManCameraAnimationComponent.h"
#include "Components/CheckDeathAreaComponent.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "OldManCharacter.generated.h"

class AOldManPullItemBase;
class AOldManInterectItemBase;
class AOldManBulletBase;
class AOldManPersonPlayerController;
class AOldManCableBase;
class UOldManAnimInstance;

USTRUCT(BlueprintType)
struct FOldManRebornData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RebornData")
    bool IfWaitInput = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RebornData")
    FVector ReBornPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RebornData")
    FRotator ReBornRotation = FRotator::ZeroRotator;
};

UCLASS()
class OLDMAN_API AOldManCharacter : public AXyCharacterBase, public IStateMachineOwner
{
    GENERATED_BODY()

public:
    AOldManCharacter(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
    virtual bool CanJumpInternal_Implementation() const override;

private:
    UPROPERTY()
    AOldManPersonPlayerController* OldManController;

    void OnMouseX(float Value);
    void OnMouseY(float Value);

#pragma region Control Param
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* InteractionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class UBoxComponent* CableDetectionBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UOldManMovementComponent* OldManMovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UOldManCameraComponent* CameraComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DeathCheck")
    UCheckDeathAreaComponent* CheckDeathAreaComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
    UStateMachineBase* StateMachine;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
    UOldManCharacterAttributes* CharacterAttributes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    UOldManCharacterAttributes* GamePadCharacterAttributes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    UOldManCharacterAttributes* KeyBoardCharacterAttributes;

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetPlayerCurMoveState(EPlayerBaseMoveState NewMoveState);
    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    EPlayerBaseMoveState GetPlayerCurMoveState() { return CurPlayerMoveState; }

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetPlayerCurActionState(EPlayerActionState NewActionState);
    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    EPlayerActionState GetPlayerActionState() { return CurPlayerActionState; }

    UPROPERTY()
    UOldManAnimInstance* AnimBlueprintClass;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FVector MovementInputVector;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasJumpInput;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasAttackInput;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementInput(FVector inputDir);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetJumpInput(bool bJumping);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(bool bRunning);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ChangeSlopeState(bool slopeState);

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetUseCustomGravity(bool CustomGravityOnEnable);

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetGravityDirection();

    FVector PerformGravityRaycast();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraOffset(const FVector& Offset);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraThirdPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraInSlopeMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraMouseCursorMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    ECameraMode GetCurrentCameraMode() const;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ShakeCamera(float Intensity, float Duration);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Animation")
    UOldManCameraAnimationComponent* CameraAnimationComponent;

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PlayCameraAnimation(const FOldManCameraAnimationData& AnimationData, bool bForceRestart);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void StopCameraAnimation(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PauseCameraAnimation();

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    bool IsCameraAnimationPlaying() const;

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void SwitchCameraAnimation(const FOldManCameraAnimationData& AnimationData, float TransitionTime);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void SetCameraAnimationTarget(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void SetCameraFollowParameters(const FVector& PositionOffset, float Distance, bool bWithRotation, bool bLookAtTarget = true);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    FRotator GetAnimationCameraRotation() const;

    UFUNCTION(BlueprintCallable, Category = "Camera")
    FRotator GetEffectiveCameraRotation() const;

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PlayFollowPlayerAnimation(const FVector& PositionOffset = FVector(-300.0f, 0.0f, 100.0f),
        bool bWithRotation = true,
        AActor* LookAtTarget = nullptr,
        const FVector& LookAtOffset = FVector::ZeroVector,
        const FRotator& CameraRotationOffset = FRotator::ZeroRotator,
        bool bUseExtensionLineOffset = true);

    UFUNCTION(BlueprintCallable, Category = "Camera|Animation")
    void PlayFollowPlayerWithMouseExposure(
        const FVector& PositionOffset,
        bool bWithRotation = true,
        bool bExposeMousePosition = true,
        bool bDisablePlayerInput = false
    );

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void UpdateFollowPlayerTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    UOldManCameraAnimationComponent* GetCameraAnimationComponent() const { return CameraAnimationComponent; }

    UFUNCTION(BlueprintCallable, Category = "Character")
    USkeletalMeshComponent* GetCharacterMesh() const { return GetMesh(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsMoving() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsFalling() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool CanDoubleJump() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool HasCable() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool HasMovementInput();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsActuallyGrounded() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetTimeSinceLastLanding() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintMovementState() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass);

    UFUNCTION(BlueprintCallable, Category = "Controller")
    AOldManPersonPlayerController* GetOldManController();

    UFUNCTION(BlueprintCallable, Category = "Detected")
    void DectedActors();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bIsRunning;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool hasIntoDoubleJump;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    float LastAttackTime;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bIsOnSlope;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bHasCable;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotationByGravity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetMovementDirectionFromCamera();

    UPROPERTY(BlueprintReadOnly, Category = "State")
    UOldManStateBase* PrePlayerState;

    UPROPERTY(BlueprintReadOnly, Category = "RebornData")
    FOldManRebornData PlayerRebornData;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
    EPlayerBaseMoveState CurPlayerMoveState = EPlayerBaseMoveState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerState")
    EPlayerActionState CurPlayerActionState = EPlayerActionState::Common;

private:
    bool InputActive = true;
    void UpdateInputActive(bool active);

    void OnInputDeviceChanged(EHardwareDevicePrimaryType InputDevice);

    float LastLandingTime;
    bool bWasFalling;

    void InitializeParam();
    void InitializeStateMachine();
    void InitializeCameraComponent();
    void InitializeAnimationCameraComponent();
    void InitializeEvent();

    // ===== 新增：从屏幕坐标执行射线检测的辅助函数 =====
    bool PerformRaycastFromScreen(const FVector2D& ScreenPos, FHitResult& OutHit, float TraceDistance = 10000.0f) const;
#pragma endregion

#pragma region Animation
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayMoveAnimation(float MovementSpeed, float Direction);

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayDoubleJumpAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayAttackAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayDeathAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation")
    void PlayLandAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayOnSlopeMoveAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayOnSlopeJumpAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayOnSlopeDoubleJumpAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayOnSlopeFallAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayFadeInSlopeAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Slope")
    void PlayFadeOutSlopeAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Cable")
    void PlayOnCableAnimation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Animation|Cable")
    void PlayOnCableHoriaontalJumpAnimation(bool isLeft);
#pragma endregion

#pragma region Item Param
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
    USceneComponent* bulletFirePos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
    TSubclassOf<AOldManBulletBase> firstKindBullet;

    UPROPERTY()
    bool InFireCoolDown = false;

    UPROPERTY()
    FTimerHandle FireTimerHandle;

public:
    UFUNCTION(BlueprintCallable)
    void FireBullet(AActor* actor);
    UFUNCTION()
    void CancelFireCoolDown();
    UFUNCTION()
    bool CanFireBullet();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float DragSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float MinMovementThreshold = 0.01f;

    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    AOldManPullItemBase* curOldManPullItem;

    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    bool bInPullState;

    UPROPERTY()
    bool bCouldPullItem = true;

public:
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void SetPullItemState(bool bPulling);
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    bool GetIfCouldPullItem();
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    AOldManPullItemBase* TryGetPullItem();

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void CheckPullItem();

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StartRightMousePull();

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StopRightMousePull();

    void HandleMouseLook(FVector2D mouseDelta);

public:
    UPROPERTY(BlueprintReadWrite, Category = "InterectItem")
    AOldManInterectItemBase* curOldManInterectItem;

public:
    UFUNCTION(BlueprintCallable, Category = "InterectItem")
    void SetCurOldManInterectItem(AOldManInterectItemBase* newItem);

    UFUNCTION(BlueprintCallable, Category = "InterectItem")
    void ClearCurOldManInterectItem();

    UFUNCTION(BlueprintCallable, Category = "InterectItem")
    void InterectCurOldManInterectItem(FOldManItemInteractData interectData);

public:
    UPROPERTY(BlueprintReadWrite, Category = "CableItem")
    AOldManCableBase* CurrentCable;
    UPROPERTY(BlueprintReadWrite, Category = "CableItem")
    AOldManCableBase* NextCable;
    UPROPERTY(BlueprintReadWrite, Category = "CableItem")
    bool IsLeftCable;

public:
    UFUNCTION(BlueprintCallable, Category = "InterectItem")
    void SetCurrentCable(AOldManCableBase* newCable);
    UFUNCTION(BlueprintCallable, Category = "InterectItem")
    void SetNextCable(AOldManCableBase* newCable, bool left);
#pragma endregion

#pragma region Character Param
public:
    UFUNCTION()
    void OnPlayerRespawn(bool IfWaitInput, FVector ReBornPosition, FRotator ReBornRotation);

    virtual bool IsAlive() override;

    UFUNCTION(BlueprintCallable, Category = "Param")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    float GetPlayerHealth() { return PlayerCurHealth; }

private:
    float PlayerCurHealth = 0.0f;
#pragma endregion
};