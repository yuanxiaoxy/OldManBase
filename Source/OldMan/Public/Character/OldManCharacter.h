#pragma once

#include "CoreMinimal.h"
#include "XyCharacter/XyCharacterBase.h"
#include "OldManCharacterAttributes.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/OldManCameraComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ItemBase/OldManInterectItemBase.h"
#include "ItemBase/OldManPullItemBase.h"
#include "OldManCharacter.generated.h"

class AOldManPullItemBase;
class AOldManInterectItemBase;
class AOldManPersonPlayerController;

UCLASS()
class OLDMAN_API AOldManCharacter : public AXyCharacterBase, public IStateMachineOwner
{
    GENERATED_BODY()

public:
    AOldManCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
    virtual bool CanJumpInternal_Implementation() const override;

private:
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
    void SetAttackInput(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(bool bRunning);

    // ========== 相机控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetCameraOffset(const FVector& Offset);

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetThirdPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFirstPersonMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void SetFreeLookMode();

    UFUNCTION(BlueprintCallable, Category = "Camera")
    void ShakeCamera(float Intensity, float Duration);

    // ========== 状态查询 ==========
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsMoving() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool IsFalling() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool CanDoubleJump() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "State")
    bool HasMovementInput() const;

    // ========== 地面检测改进 ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsActuallyGrounded() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetTimeSinceLastLanding() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintMovementState() const;

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

    // ========== 角色设置 ==========
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass);

    // ========== 战斗系统 ==========
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttackDetection();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackHit(AActor* HitActor);

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

    // ========== 旋转控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection);

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
#pragma endregion

#pragma region Item Param
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
#pragma endregion
};