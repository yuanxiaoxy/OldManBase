// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIConfigDataAsset.h"

bool UUIConfigDataAsset::GetUIConfig(FName UIName, FUIConfigData& OutConfig) const
{
    if (!UIConfigTable)
    {
        UE_LOG(LogTemp, Error, TEXT("UUIConfigDataAsset::GetUIConfig - UIConfigTable is null"));
        return false;
    }

    FUIConfigData* Config = UIConfigTable->FindRow<FUIConfigData>(UIName, TEXT("GetUIConfig"));
    if (Config)
    {
        OutConfig = *Config;
        return true;
    }

    return false;
}

TArray<FUIConfigData> UUIConfigDataAsset::GetAllUIConfigs() const
{
    TArray<FUIConfigData> Configs;

    if (!UIConfigTable)
    {
        return Configs;
    }

    // 获取数据表的所有行
    TArray<FUIConfigData*> AllConfigs;
    UIConfigTable->GetAllRows(TEXT("GetAllUIConfigs"), AllConfigs);

    for (FUIConfigData* Config : AllConfigs)
    {
        if (Config)
        {
            Configs.Add(*Config);
        }
    }

    return Configs;
}

TArray<FUIConfigData> UUIConfigDataAsset::GetPreloadUIConfigs() const
{
    TArray<FUIConfigData> PreloadConfigs;
    TArray<FUIConfigData> AllConfigs = GetAllUIConfigs();

    for (const FUIConfigData& Config : AllConfigs)
    {
        if (Config.bPreload)
        {
            PreloadConfigs.Add(Config);
        }
    }

    return PreloadConfigs;
}