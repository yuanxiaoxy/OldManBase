// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"   // 必须包含，否则 USaveGame 未定义
#include "SingletonBase/SingletonBase.h"
#include "xyLanguageManager.generated.h"

UENUM(BlueprintType)
enum class ELanguageType : uint8
{
    Chinese UMETA(DisplayName = "中文"),
    English UMETA(DisplayName = "English")
};

// 存档类 - 必须放在生成代码之前，且包含 UCLASS()
UCLASS()
class XYFRAME_API ULanguageSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY()
    ELanguageType SavedLanguage = ELanguageType::Chinese;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLanguageChanged, ELanguageType, NewLanguage);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UxyLanguageManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UxyLanguageManager)

public:
    UFUNCTION(BlueprintCallable, Category = "Language")
    void InitializeLanguageManager();

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Language")
    static UxyLanguageManager* GetLanguageManager() { return GetInstance(); }

    UFUNCTION(BlueprintCallable, Category = "Language")
    ELanguageType GetCurrentLanguage() const { return CurrentLanguage; }

    UFUNCTION(BlueprintCallable, Category = "Language")
    void SetCurrentLanguage(ELanguageType NewLanguage, bool bSaveToLocal = true);

    UFUNCTION(BlueprintCallable, Category = "Language")
    FString GetLanguageCode(ELanguageType Language) const;

    UFUNCTION(BlueprintCallable, Category = "Language")
    bool SaveLanguageSetting();

    UFUNCTION(BlueprintCallable, Category = "Language")
    void LoadLanguageSetting();

    UPROPERTY(BlueprintAssignable, Category = "Language")
    FOnLanguageChanged OnLanguageChanged;

private:
    ELanguageType CurrentLanguage;

    static const FString SaveSlotName;
    static const int32 SaveUserIndex;
};