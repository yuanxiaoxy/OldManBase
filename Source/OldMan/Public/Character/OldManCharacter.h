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
    // ����Ľ�ɫ������ָ��
    UPROPERTY()
    AOldManPersonPlayerController* OldManController;

    // ��ȡ��ɫ���������
    UFUNCTION(BlueprintCallable, Category = "Controller")
    AOldManPersonPlayerController* GetOldManController();

#pragma region Control Param
public:
    // ========== ������ ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UOldManCameraComponent* CameraComponent;

    // ========== ״̬�� ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
    UStateMachineBase* StateMachine;

    // ========== ��ɫ���� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes")
    UOldManCharacterAttributes* CharacterAttributes;

    // ========== ���뻺�� ==========
    UPROPERTY(BlueprintReadWrite, Category = "Input")
    FVector MovementInputVector;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasJumpInput;

    UPROPERTY(BlueprintReadWrite, Category = "Input")
    bool bHasAttackInput;

    // ========== �ƶ����ƽӿ� ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementInput(FVector inputDir);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetJumpInput(bool bJumping);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetAttackInput(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetRunning(bool bRunning);

    // ========== ������� ==========
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

    // ========== ״̬��ѯ ==========
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

    // ========== ��ؼ��Ľ� ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    bool IsActuallyGrounded() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    float GetTimeSinceLastLanding() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintMovementState() const;

    // ========== �����ӿ� ==========
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

    // ========== ģ������ ==========
    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass);

    // ========== ս��ϵͳ ==========
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttackDetection();

    UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
    void OnAttackHit(AActor* HitActor);

    // ========== ������� ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    float CameraDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FVector CameraOffset = FVector(0.0f, 0.0f, 75.0f);

    // ========== ״̬���� ==========
    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bIsRunning;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool hasIntoDoubleJump;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    float LastAttackTime;

    // ========== �ڲ����� ==========
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    FVector GetMovementDirectionFromCamera() const;

private:
    // ��ؼ��Ľ�
    float LastLandingTime;
    bool bWasFalling;

    // ��ʼ������
    void InitializeParam();
    void InitializeStateMachine();
    void InitializeCameraComponent();
#pragma endregion

#pragma region Item Param
public:
    //��ʱ����
    FVector2D CurrentMouseDelta;
    // �϶�������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float DragSensitivity = 0.001f;

    // ��С�ƶ���ֵ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drag")
    float MinMovementThreshold = 0.01f;

    FVector2D LastMousePosition;
    bool bHasValidLastPosition;









//PullItem
public:
    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    AOldManPullItemBase* curOldManPullItem;

    UPROPERTY(BlueprintReadWrite, Category = "PullItem")
    bool bHasPullItem;

public:
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void SetPullItemState(bool bAttacking);

    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StartRightMousePull();
    UFUNCTION(BlueprintCallable, Category = "PullItem")
    void StopRightMousePull();

//InterectItem
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