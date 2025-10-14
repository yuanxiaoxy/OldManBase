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
    void HandleMovement(float DeltaTime);
    void HandleRotation(float DeltaTime);
    void ApplyMovement(const FVector& Direction, float Speed);
    void Jump();

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
    void ResetJumpInput(bool jumpInputActive);
    void ResetDoubleJump(bool couldBDoubleJump);
    bool HasAttackInput();
    bool IsRunning();

    // �¼�����
    void InPatchEvents();
    void OutPatchEvents();

private:
    FName Key_CheckJumpStatesTranisition = "key_CheckJumpStatesTranisition";
    FName Key_CheckAttackStatesTranisition = "key_CheckAttackStatesTranisition";
};