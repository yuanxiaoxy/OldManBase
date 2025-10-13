#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EventManager/MyEventManager.h"
#include "MonoManager/MonoManager.h"
#include "XyPlayerControllerBase.generated.h"

UCLASS(Abstract, Blueprintable)
class XYFRAME_API AXyPlayerControllerBase : public APlayerController
{
    GENERATED_BODY()

public:
    AXyPlayerControllerBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void SetupInputComponent() override;

public:
    // ========== 输入系统 ==========

    // 绑定输入
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void BindCharacterInputs();

    // 切换输入启用状态
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void SetInputEnabled(bool bEnabled);

    // ========== 角色控制 ==========

    // 获取控制的角色（优化版本）
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyPlayerController")
    class AXyCharacterBase* GetXyCharacter() const;

    // 重生角色
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RespawnCharacter();

    // ========== 事件处理 ==========

    // 注册事件监听
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RegisterEventListeners();

    // 移除事件监听
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void UnregisterEventListeners();

protected:
    // ========== 输入处理函数 ==========

    // 移动输入
    UFUNCTION()
    virtual void HandleMoveForward(float Value);

    UFUNCTION()
    virtual void HandleMoveRight(float Value);

    // 视角输入
    UFUNCTION()
    virtual void HandleLookUp(float Value);

    UFUNCTION()
    virtual void HandleTurn(float Value);

    // 动作输入
    UFUNCTION()
    virtual void HandleJump();

    UFUNCTION()
    virtual void HandleStopJumping();

    UFUNCTION()
    virtual void HandleAttack();

    // ========== 事件回调 ==========

    // 角色事件回调
    UFUNCTION()
    virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData);

    // 处理角色死亡
    UFUNCTION()
    virtual void HandleCharacterDeath();

    // ========== 框架集成 ==========

    UMyEventManager* GetEventManager() const { return UMyEventManager::GetEventManager(); }
    UMonoManager* GetMonoManager() const { return UMonoManager::GetMonoManager(); }

protected:
    // 输入是否启用
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    bool bInputEnabled;

    // 鼠标灵敏度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float MouseSensitivity;

    // 控制器灵敏度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float ControllerSensitivity;

private:
    // 输入组件缓存
    UInputComponent* CachedInputComponent;

    // 缓存角色指针避免频繁 Cast（性能优化）
    UPROPERTY()
    mutable class AXyCharacterBase* CachedXyCharacter;

    // 缓存有效性标志
    mutable bool bCachedCharacterValid;
};