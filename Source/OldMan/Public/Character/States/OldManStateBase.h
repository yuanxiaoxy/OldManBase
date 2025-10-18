#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "OldManStateBase.generated.h"

UCLASS()
class OLDMAN_API UOldManStateBase : public UStateBase
{
    GENERATED_BODY()

public:
    virtual void Enter() override;
    virtual void Exit() override;
    virtual void Update(float DeltaTime) override;

protected:
    // ����Ľ�ɫָ��
    UPROPERTY()
    class AOldManCharacter* CachedOldManCharacter;

    // �ƶ���ط���
    virtual void HandleMovement(float DeltaTime);
    void HandleRotation(float DeltaTime);
    void ApplyMovement(const FVector& Direction, float Speed);
    void Jump();
    void HandleMovementInAir(float DeltaTime);

    // ״̬��鷽��
    bool CheckDeathCondition();
    bool CheckFallingCondition();
    bool CheckJumpCondition();
    bool CheckAttackCondition();

    // ��������
    class AOldManCharacter* GetOldManCharacter();
    class UCharacterMovementComponent* GetCharacterMovement();

    // ����״̬
    bool HasMovementInput();
    bool HasJumpInput();
    bool HasAttackInput();
    bool IsRunning();
    void ResetJumpInput(bool jumpInputActive);

    // �¼�����
    void InPatchEvents();
    void OutPatchEvents();

    //��ʱ����
    UPROPERTY()
    float targetSpeed;

private:
    FName Key_CheckJumpStatesTranisition = "key_CheckJumpStatesTranisition";
    FName Key_CheckAttackStatesTranisition = "key_CheckAttackStatesTranisition";
};