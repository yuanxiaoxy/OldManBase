#pragma once

#include "CoreMinimal.h"
#include "StateMachine/StateMachineBase.h"
#include "OldManStateBase.generated.h"

// 状态转换条件委托
DECLARE_DELEGATE_RetVal(bool, FStateTransitionCondition);

// 状态转换规则
USTRUCT()
struct FStateTransitionRule
{
    GENERATED_BODY()

    UPROPERTY()
    TSubclassOf<UStateBase> TargetState;

    FStateTransitionCondition Condition;

    FString DebugName;
};

// 简化添加转换规则的宏
#define ADD_TRANSITION(TargetStateClass, ConditionMethod) \
    AddTransitionRule(TargetStateClass::StaticClass(), \
    FStateTransitionCondition::CreateUObject(this, &ThisClass::ConditionMethod), \
    TEXT(#ConditionMethod))

#define ADD_LAMBDA_TRANSITION(TargetStateClass, LambdaExpr, DebugName) \
    AddTransitionRule(TargetStateClass::StaticClass(), \
    FStateTransitionCondition::CreateLambda(LambdaExpr), \
    TEXT(DebugName))

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

    // 状态转换系统
    virtual void SetupTransitionRules();
    void AddTransitionRule(TSubclassOf<UStateBase> TargetState, FStateTransitionCondition Condition, const FString& DebugName = "");
    void CheckAllTransitions();

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
    bool CheckPullItemStateCondition();
    bool CheckOnSlopeCondition();

    // 辅助方法
    class AOldManCharacter* GetOldManCharacter();
    class UCharacterMovementComponent* GetCharacterMovement();
    bool HasMovementInput();
    bool HasJumpInput();
    bool HasAttackInput();
    bool IsRunning();
    void ResetJumpInput(bool jumpInputActive);

    // 事件管理
    void InPatchEvents();
    void OutPatchEvents();

    // 转换规则列表
    UPROPERTY()
    TArray<FStateTransitionRule> TransitionRules;

    UPROPERTY()
    float targetSpeed;
};