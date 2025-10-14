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
    // 缓存的角色指针
    UPROPERTY()
    class AOldManCharacter* CachedOldManCharacter;

    // 移动相关方法
    void HandleMovement(float DeltaTime);
    void HandleRotation(float DeltaTime);
    void ApplyMovement(const FVector& Direction, float Speed);
    void Jump();

    // 状态检查方法
    bool CheckDeathCondition();
    bool CheckFallingCondition();
    bool CheckJumpCondition();
    bool CheckAttackCondition();

    // 辅助方法
    class AOldManCharacter* GetOldManCharacter();
    class UCharacterMovementComponent* GetCharacterMovement();

    // 输入状态
    bool HasMovementInput();
    bool HasJumpInput();
    void ResetJumpInput(bool jumpInputActive);
    void ResetDoubleJump(bool couldBDoubleJump);
    bool HasAttackInput();
    bool IsRunning();

    // 事件管理
    void InPatchEvents();
    void OutPatchEvents();

private:
    FName Key_CheckJumpStatesTranisition = "key_CheckJumpStatesTranisition";
    FName Key_CheckAttackStatesTranisition = "key_CheckAttackStatesTranisition";
};