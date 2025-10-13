#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h" // ������ͷ�ļ�
#include "EventManager/MyEventManager.h"
#include "ResourceManager/ResourceManager.h"
#include "MonoManager/MonoManager.h"
#include "XyCharacterBase.generated.h"

// ��ɫ״̬ö��
UENUM(BlueprintType)
enum class EXyCharacterState : uint8
{
    Idle UMETA(DisplayName = "����"),
    Moving UMETA(DisplayName = "�ƶ�"),
    Jumping UMETA(DisplayName = "��Ծ"),
    Attacking UMETA(DisplayName = "����"),
    Dead UMETA(DisplayName = "����")
};

// ��ɫ����
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
    virtual void Tick(float DeltaTime) override; // ��� Tick ����

public:
    // ========== �������� ==========

    // ��ʼ����ɫ
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void InitializeCharacter(const FCharacterData& InData);

    // ��ȡ��ɫ����
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    FCharacterData GetCharacterData() const { return CharacterData; }

    // ��ȡ��ǰ״̬
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    EXyCharacterState GetCharacterState() const { return CurrentState; }

    // ========== ״̬���� ==========

    // �ı��ɫ״̬
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual bool ChangeState(EXyCharacterState NewState);

    // ״̬���
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    bool IsAlive() const { return CurrentState != EXyCharacterState::Dead && CharacterData.Health > 0; }

    // ========== ����ֵϵͳ ==========

    // Ӧ���˺�
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void ApplyDamage(float Damage, AActor* DamageCauser = nullptr);

    // ����
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void ApplyHeal(float HealAmount);

    // ��ȡ����ֵ�ٷֱ�
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyCharacter")
    float GetHealthPercent() const { return CharacterData.MaxHealth > 0 ? CharacterData.Health / CharacterData.MaxHealth : 0; }

    // ========== ��Դ���� ==========

    // �첽���ؽ�ɫ��Դ
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void LoadCharacterResources();

    // ж�ؽ�ɫ��Դ
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void UnloadCharacterResources();

    // ========== �¼�ϵͳ ==========

    // ������ɫ�¼�
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    virtual void TriggerCharacterEvent(EGameEventType EventType, const FGameEventData& EventData);

    // �����򵥽�ɫ�¼�
    UFUNCTION(BlueprintCallable, Category = "XyCharacter")
    void TriggerSimpleCharacterEvent(EGameEventType EventType, const FString& TextParam = "", float ValueParam = 0.0f);

protected:
    // ========== �ܱ������麯�� ==========

    // ״̬����
    virtual void OnEnterState(EXyCharacterState NewState);

    // ״̬�˳�
    virtual void OnExitState(EXyCharacterState OldState);

    // ״̬����
    virtual void OnUpdateState(float DeltaTime);

    // ��Դ������ɻص�
    UFUNCTION()
    virtual void OnCharacterResourcesLoaded(UObject* LoadedResource);

    // ��������
    virtual void HandleDeath();

    // ��������
    virtual void HandleRespawn();

    // ========== ��ܼ��� ==========

    // ��ȡ��ܹ�����
    UMyEventManager* GetEventManager() const { return UMyEventManager::GetEventManager(); }
    UResourceManager* GetResourceManager() const { return UResourceManager::GetResourceManager(); }
    UMonoManager* GetMonoManager() const { return UMonoManager::GetMonoManager(); }

protected:
    // ========== ���� ==========

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FCharacterData CharacterData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
    EXyCharacterState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterMeshPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterAnimBlueprintPath;

    // ��Դ��������ID
    FString ResourceLoadRequestId;

    // ������ʱ��ID
    FString RespawnTimerId;

private:
    // �ڲ�״̬����
    void InternalUpdateState(float DeltaTime);
};