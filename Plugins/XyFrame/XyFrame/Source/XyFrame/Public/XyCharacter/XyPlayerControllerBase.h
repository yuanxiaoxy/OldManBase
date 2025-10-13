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
    // ========== ����ϵͳ ==========

    // ������
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void BindCharacterInputs();

    // �л���������״̬
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void SetInputEnabled(bool bEnabled);

    // ========== ��ɫ���� ==========

    // ��ȡ���ƵĽ�ɫ���Ż��汾��
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XyPlayerController")
    class AXyCharacterBase* GetXyCharacter() const;

    // ������ɫ
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RespawnCharacter();

    // ========== �¼����� ==========

    // ע���¼�����
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void RegisterEventListeners();

    // �Ƴ��¼�����
    UFUNCTION(BlueprintCallable, Category = "XyPlayerController")
    virtual void UnregisterEventListeners();

protected:
    // ========== ���봦���� ==========

    // �ƶ�����
    UFUNCTION()
    virtual void HandleMoveForward(float Value);

    UFUNCTION()
    virtual void HandleMoveRight(float Value);

    // �ӽ�����
    UFUNCTION()
    virtual void HandleLookUp(float Value);

    UFUNCTION()
    virtual void HandleTurn(float Value);

    // ��������
    UFUNCTION()
    virtual void HandleJump();

    UFUNCTION()
    virtual void HandleStopJumping();

    UFUNCTION()
    virtual void HandleAttack();

    // ========== �¼��ص� ==========

    // ��ɫ�¼��ص�
    UFUNCTION()
    virtual void OnCharacterEvent(EGameEventType EventType, const FGameEventData& EventData);

    // �����ɫ����
    UFUNCTION()
    virtual void HandleCharacterDeath();

    // ========== ��ܼ��� ==========

    UMyEventManager* GetEventManager() const { return UMyEventManager::GetEventManager(); }
    UMonoManager* GetMonoManager() const { return UMonoManager::GetMonoManager(); }

protected:
    // �����Ƿ�����
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    bool bInputEnabled;

    // ���������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float MouseSensitivity;

    // ������������
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controller")
    float ControllerSensitivity;

private:
    // �����������
    UInputComponent* CachedInputComponent;

    // �����ɫָ�����Ƶ�� Cast�������Ż���
    UPROPERTY()
    mutable class AXyCharacterBase* CachedXyCharacter;

    // ������Ч�Ա�־
    mutable bool bCachedCharacterValid;
};