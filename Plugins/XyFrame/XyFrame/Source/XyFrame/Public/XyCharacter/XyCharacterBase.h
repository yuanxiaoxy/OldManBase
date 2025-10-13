#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h" // 添加这个头文件
#include "EventManager/MyEventManager.h"
#include "ResourceManager/ResourceManager.h"
#include "MonoManager/MonoManager.h"
#include "XyCharacterBase.generated.h"

// 角色状态枚举
UENUM(BlueprintType)
enum class EXyCharacterState : uint8
{
    Idle UMETA(DisplayName = "闲置"),
    Moving UMETA(DisplayName = "移动"),
    Jumping UMETA(DisplayName = "跳跃"),
    Attacking UMETA(DisplayName = "攻击"),
    Dead UMETA(DisplayName = "死亡")
};

// 角色数据
USTRUCT(BlueprintType)
struct FCharacterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float MoveSpeed;

    FCharacterData()
        : CharacterName(TEXT("Unknown"))
        , Level(1)
        , Health(100.0f)
        , MaxHealth(100.0f)
        , MoveSpeed(600.0f)
    {
    }
};

UCLASS(Abstract, Blueprintable)
class XYFRAME_API AXyCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    AXyCharacterBase();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void UnPossessed() override;
    virtual void Tick(float DeltaTime) override; // 添加 Tick 声明

public:
    // ========== 基础功能 ==========

    // 初始化角色
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void InitializeCharacter(const FCharacterData& InData);

    // 获取角色数据
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    FCharacterData GetCharacterData() const { return CharacterData; }

    // 获取当前状态
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    EXyCharacterState GetCharacterState() const { return CurrentState; }

    // ========== 状态管理 ==========

    // 改变角色状态
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual bool ChangeState(EXyCharacterState NewState);

    // 状态检查
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    bool IsAlive() const { return CurrentState != EXyCharacterState::Dead && CharacterData.Health > 0; }

    // ========== 生命值系统 ==========

    // 应用伤害
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void ApplyDamage(float Damage, AActor* DamageCauser = nullptr);

    // 治疗
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void ApplyHeal(float HealAmount);

    // 获取生命值百分比
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    float GetHealthPercent() const { return CharacterData.MaxHealth > 0 ? CharacterData.Health / CharacterData.MaxHealth : 0; }

    // ========== 资源管理 ==========

    // 异步加载角色资源
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void LoadCharacterResources();

    // 卸载角色资源
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void UnloadCharacterResources();

    // ========== 事件系统 ==========

    // 触发角色事件
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void TriggerCharacterEvent(EGameEventType EventType, const FGameEventData& EventData);

    // 触发简单角色事件
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    void TriggerSimpleCharacterEvent(EGameEventType EventType, const FString& TextParam = "", float ValueParam = 0.0f);

protected:
    // ========== 受保护的虚函数 ==========

    // 状态进入
    virtual void OnEnterState(EXyCharacterState NewState);

    // 状态退出
    virtual void OnExitState(EXyCharacterState OldState);

    // 状态更新
    virtual void OnUpdateState(float DeltaTime);

    // 资源加载完成回调
    UFUNCTION()
    virtual void OnCharacterResourcesLoaded(UObject* LoadedResource);

    // 死亡处理
    virtual void HandleDeath();

    // 重生处理
    virtual void HandleRespawn();

    // ========== 框架集成 ==========

    // 获取框架管理器
    UMyEventManager* GetEventManager() const { return UMyEventManager::GetEventManager(); }
    UResourceManager* GetResourceManager() const { return UResourceManager::GetResourceManager(); }
    UMonoManager* GetMonoManager() const { return UMonoManager::GetMonoManager(); }

protected:
    // ========== 属性 ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FCharacterData CharacterData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    EXyCharacterState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterMeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterAnimBlueprintPath;

    // 资源加载请求ID
    FString ResourceLoadRequestId;

    // 重生计时器ID
    FString RespawnTimerId;

private:
    // 内部状态处理
    void InternalUpdateState(float DeltaTime);
};