// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "UITypes.h"  // 包含类型定义
#include "UIConfigDataAsset.generated.h"

// UI配置数据结构
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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    bool bPreload = false;

    FUIConfigData()
        : DefaultLayer(EUIPanelLayer::Middle)
        , bPreload(false)
    {
    }
};

// UI配置数据资产
UCLASS(BlueprintType)
class XYFRAME_API UUIConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UDataTable* UIConfigTable;

    // 获取UI配置
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool GetUIConfig(FName UIName, FUIConfigData& OutConfig) const;

    // 获取所有UI配置
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetAllUIConfigs() const;

    // 获取需要预加载的UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetPreloadUIConfigs() const;
};