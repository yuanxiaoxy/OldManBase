// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UITypes.h"
#include "UIManager.generated.h"

// ǰ������
class UUIConfigDataAsset;
class UUIBase;
struct FUIConfigData;  // ������ǰ������

// UI��Ϣ�ṹ
USTRUCT(BlueprintType)
struct FUIInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FName UIName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UUserWidget> WidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIPanelLayer Layer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UUserWidget* WidgetInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bIsPreloaded;

    FUIInfo()
        : Layer(EUIPanelLayer::Middle)
        , State(EUIState::Hidden)
        , WidgetInstance(nullptr)
        , bIsPreloaded(false)
    {
    }
};

// UI�¼�
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIShown, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIHidden, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIClosed, FName, UIName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIConfigLoaded, bool, bSuccess);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UUIManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UUIManager)

public:
    // ��ʼ��UI������
    UFUNCTION(BlueprintCallable, Category = "UI")
    void InitializeUIManager(UUIConfigDataAsset* ConfigDataAsset = nullptr);

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI", meta = (DisplayName = "Get UI Manager"))
    static UUIManager* GetUIManager() { return GetInstance(); }

    UUIManager();
    virtual ~UUIManager() override;

    // ========== UI�������ڷ��� ==========

    // ��ʾUI����
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUI(TSubclassOf<UUserWidget> WidgetClass, EUIPanelLayer Layer = EUIPanelLayer::Middle, UObject* Data = nullptr, FName UIName = "");

    // ����������ʾUI
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* ShowUIByName(FName UIName, UObject* Data = nullptr);

    // ����UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideUI(FName UIName);

    // �ر�UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseUI(FName UIName);

    // �ر�����UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CloseAllUI();

    // ========== ���÷��� ==========

    // ����UI����
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void LoadUIConfig(UUIConfigDataAsset* ConfigDataAsset);

    // ���¼���UI����
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void ReloadUIConfig();

    // Ԥ����UI
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadUIs(const TArray<FName>& UINames);

    // Ԥ�������б��ΪԤ���ص�UI
    UFUNCTION(BlueprintCallable, Category = "UI|Config")
    void PreloadMarkedUIs();

    // ========== UI��Ϣ��ѯ ==========

    // ��ȡUIʵ��
    UFUNCTION(BlueprintCallable, Category = "UI")
    UUserWidget* GetUI(FName UIName) const;

    // ���UI�Ƿ���ʾ��
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool IsUIVisible(FName UIName) const;

    // ���UI�Ƿ����
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool DoesUIExist(FName UIName) const;

    // ��ȡ���л�Ծ��UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllActiveUIs() const;

    // ��ȡ����ע���UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FName> GetAllRegisteredUIs() const;

    // ========== �㼶���� ==========

    // ����UI�㼶
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetUILayer(FName UIName, EUIPanelLayer NewLayer);

    // ��ȡUI�㼶
    UFUNCTION(BlueprintCallable, Category = "UI")
    EUIPanelLayer GetUILayer(FName UIName) const;

    // ========== ������Ч�� ==========

    // ����Ч��
    UFUNCTION(BlueprintCallable, Category = "UI")
    void FadeInUI(FName UIName, float Duration = 0.3f);

    // ����Ч��
    UFUNCTION(BlueprintCallable, Category = "UI")
    void FadeOutUI(FName UIName, float Duration = 0.3f);

    // ========== ���Թ��� ==========

    // ��ӡ����UI��Ϣ
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintAllUIs();

    // ��ӡ�㼶��Ϣ
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintLayerInfo();

    // ��ӡ������Ϣ
    UFUNCTION(BlueprintCallable, Category = "UI|Debug")
    void PrintConfigInfo();

    // ========== �¼� ==========

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIShown OnUIShown;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIHidden OnUIHidden;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIClosed OnUIClosed;

    UPROPERTY(BlueprintAssignable, Category = "UI|Events")
    FOnUIConfigLoaded OnUIConfigLoaded;

private:
    // UIע���
    UPROPERTY()
    TMap<FName, FUIInfo> UIRegistry;

    // �㼶���
    UPROPERTY()
    TMap<EUIPanelLayer, UCanvasPanel*> LayerPanels;

    // ��������
    UPROPERTY()
    UUIConfigDataAsset* UIConfigData;

    // ����������
    UPROPERTY()
    UWorld* WorldContext;

    // ������
    UPROPERTY()
    UCanvasPanel* RootCanvas;

    // ˽�з���
    void CreateLayerPanels();
    UCanvasPanel* GetOrCreateLayerPanel(EUIPanelLayer Layer);
    void AddToLayer(UUserWidget* Widget, EUIPanelLayer Layer);
    void RemoveFromLayer(UUserWidget* Widget);

    // ���ù�����
    bool RegisterUIFromConfig(const FUIConfigData& Config);  // �޸������ӷ�������
    void RegisterAllUIsFromConfig();
    TSubclassOf<UUserWidget> LoadWidgetClass(const TSoftClassPtr<UUserWidget>& SoftClassPtr);

    // ��������
    void HandleFadeAnimation(FName UIName, float TargetAlpha, float Duration);

    // ��ȡ����
    UWorld* GetWorld() const override;
};