// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "UITypes.h"
#include "LanguageManager/xyLanguageManager.h"
#include "UIConfigDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FUIConfigData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FName UIName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSoftClassPtr<UUserWidget> WidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIPanelLayer DefaultLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIInputMode DefaultInputMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bShowMouseCursor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSoftObjectPtr<UInputMappingContext> DefaultInputMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    int32 InputPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bPreload = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    EUIPanelType PanelType = EUIPanelType::Other;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bModifyInput = true;

    // ========== 新增：UI 优先级（数值越大层级越高）==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    int32 Priority = 0;

    // ========== 多语言 Widget 映射 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Localization")
    TMap<ELanguageType, TSoftClassPtr<UUserWidget>> LocalizedWidgetClasses;

    TSubclassOf<UUserWidget> GetLocalizedWidgetClass() const;

    FUIConfigData()
        : DefaultLayer(EUIPanelLayer::Middle)
        , DefaultInputMode(EUIInputMode::UIOnly)
        , bShowMouseCursor(true)
        , InputPriority(0)
        , bPreload(false)
        , bModifyInput(true)
        , Priority(0)
    {
    }
};

UCLASS(BlueprintType)
class XYFRAME_API UUIConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UDataTable* UIConfigTable;

    UFUNCTION(BlueprintCallable, Category = "UI")
    bool GetUIConfig(FName UIName, FUIConfigData& OutConfig) const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetAllUIConfigs() const;

    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetPreloadUIConfigs() const;
};