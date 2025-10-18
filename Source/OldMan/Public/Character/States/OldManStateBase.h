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
    virtual void HandleMovement(float DeltaTime);
    void HandleRotation(float DeltaTime);
    void ApplyMovement(const FVector& Direction, float Speed);
    void Jump();
    void HandleMovementInAir(float DeltaTime);

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
    bool HasAttackInput();
    bool IsRunning();
    void ResetJumpInput(bool jumpInputActive);

    // 事件管理
    void InPatchEvents();
    void OutPatchEvents();

    //临时数据
    UPROPERTY()
    float targetSpeed;

private:
    FName Key_CheckJumpStatesTranisition = "key_CheckJumpStatesTranisition";
    FName Key_CheckAttackStatesTranisition = "key_CheckAttackStatesTranisition";
};