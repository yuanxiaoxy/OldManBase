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
#include "OldManCharacter.generated.h"

class AOldManPullItemBase;
class AOldManInterectItemBase;
class AOldManBulletBase;
class AOldManPersonPlayerController;
class AOldManCableBase;

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
    //是否断掉玩家输入相关
    //是否执行Update
    bool Active = true;
    //改变Active的值
    void ChangeInputActive(bool active);

    // 玩家控制器引用
    UPROPERTY()
    AOldManPersonPlayerController* OldManController;

    // 获取玩家控制器
    UFUNCTION(BlueprintCallable, Category = "Controller")
    AOldManPersonPlayerController* GetOldManController();

    // 鼠标输入处理
    void OnMouseX(float Value);
    void OnMouseY(float Value);

#pragma region Control Param
public:
    // 用于互动的碰撞组件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UBoxComponent* InteractionBox;

    // 绳索检测碰撞器 - 使用Box放在脚下
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cable")
    class UBoxComponent* CableDetectionBox;

    //角色移动组件
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UOldManMovementComponent* OldManMovementComponent;

    // ========== 相机组件 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UOldManCameraComponent* CameraComponent;

    // ========== 状态机 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
    UStateMachineBase* StateMachine;

    // ========== 角色属性 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    UOldManCharacterAttributes* CharacterAttributes;

    // ========== 输入控制 ==========
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FVector MovementInputVector;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasJumpInput;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasAttackInput;

    // ========== 移动控制函数 ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementInput(FVector inputDir);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetJumpInput(bool bJumping);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(bool bRunning);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void ChangeSlopeState(bool slopeState);

    // ========== 重力控制 ==========

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetUseCustomGravity(bool CustomGravityOnEnable);

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void SetGravityDirection();

    FVector PerformGravityRaycast();

    // ========== 相机控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraOffset(const FVector& Offset);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraThirdPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraInSlopeMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ShakeCamera(float Intensity, float Duration);

    // ========== 相机动画组件 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Animation")
    UOldManCameraAnimationComponent* CameraAnimationComponent;

    // ========== 相机动画控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PlayCameraAnimation(const FOldManCameraAnimationData& AnimationData);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void StopCameraAnimation(bool bImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PauseCameraAnimation();

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    bool IsCameraAnimationPlaying() const;

    // 新增：运行时设置相机动画目标
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void SetCameraAnimationTarget(AActor* TargetActor);

    // 新增：设置跟随参数
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void SetCameraFollowParameters(const FVector& PositionOffset, float Distance, bool bWithRotation, bool bLookAtTarget = true);

    // 新增：获取动画相机旋转（用于移动方向计算）
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    FRotator GetAnimationCameraRotation() const;

    // 新增：获取当前有效的相机旋转（根据是否播放动画相机）
    UFUNCTION(BlueprintCallable, Category = "Camera")
    FRotator GetEffectiveCameraRotation() const;

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void PlayFollowPlayerAnimation(const FVector& PositionOffset = FVector(-300.0f, 0.0f, 100.0f),
        bool bWithRotation = true,
        AActor* LookAtTarget = nullptr,
        const FVector& LookAtOffset = FVector::ZeroVector,
        const FRotator& CameraRotationOffset = FRotator::ZeroRotator,
        bool bUseExtensionLineOffset = true);

    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    void UpdateFollowPlayerTarget(AActor* NewTarget);

    // 新增：获取相机动画组件
    UFUNCTION(BlueprintCallable, Category = "Camera Animation")
    UOldManCameraAnimationComponent* GetCameraAnimationComponent() const { return CameraAnimationComponent; }
    
    // 获取角色网格组件
    UFUNCTION(BlueprintCallable, Category = "Character")
    USkeletalMeshComponent* GetCharacterMesh() const { return GetMesh(); }

    // ========== 状态查询 ==========
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsMoving() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsFalling() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool CanDoubleJump() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool HasCable() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool HasMovementInput() const;

    // ========== 地面检测改进 ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsActuallyGrounded() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetTimeSinceLastLanding() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintMovementState() const;

    // ========== 角色设置 ==========
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass);

    // ========== 鼠标左键物体检测 ==========
    UFUNCTION(BlueprintCallable, Category = "Detected")
    void DectedActors();

    // ========== 相机属性 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    // ========== 状态变量 ==========
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

    // ========== 旋转控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotationByGravity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetMovementDirectionFromCamera() const;

private:
    // 地面检测改进
    float LastLandingTime;
    bool bWasFalling;

    // 初始化参数
    void InitializeParam();
    void InitializeStateMachine();
    void InitializeCameraComponent();
    void InitializeAnimationCameraComponent();
    void InitializeEvent();
#pragma endregion

#pragma region Animation
public:
    // ========== 动画事件 ==========
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
    //Bullet
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
    USceneComponent* bulletFirePos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
    TSubclassOf<AOldManBulletBase> firstKindBullet;

public:
    UFUNCTION(BlueprintCallable)
    void FireBullet(AActor* actor);

    // PullItem
public:
    // 拖动灵敏度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float DragSensitivity = 0.5f;

    // 最小移动阈值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float MinMovementThreshold = 0.01f;

    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    AOldManPullItemBase* curOldManPullItem;

    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    bool bInCanPullState;

public:
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void SetPullItemState(bool bPulling);

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StartRightMousePull();

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StopRightMousePull();

    void HandleMouseLook(FVector2D mouseDelta);

    // InterectItem
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

    //Cable Item
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
    void OnPlayerRespawn(FGameEventData EventData);

    virtual bool IsAlive() override;

    // 受击相关函数
    UFUNCTION(BlueprintCallable, Category = "Param")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    float GetPlayerHealth() {return PlayerCurHealth;}

private:
    float PlayerCurHealth = 0.0f;
#pragma endregion
};