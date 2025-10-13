// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "UITypes.h"  // �������Ͷ���
#include "UIConfigDataAsset.generated.h"

// UI�������ݽṹ
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

// UI���������ʲ�
UCLASS(BlueprintType)
class XYFRAME_API UUIConfigDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    UDataTable* UIConfigTable;

    // ��ȡUI����
    UFUNCTION(BlueprintCallable, Category = "UI")
    bool GetUIConfig(FName UIName, FUIConfigData& OutConfig) const;

    // ��ȡ����UI����
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetAllUIConfigs() const;

    // ��ȡ��ҪԤ���ص�UI
    UFUNCTION(BlueprintCallable, Category = "UI")
    TArray<FUIConfigData> GetPreloadUIConfigs() const;
};