// Fill out your copyright notice in the Description page of Project Settings.

#include "UIManager/UIConfigDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "LanguageManager/xyLanguageManager.h"

static TSubclassOf<UUserWidget> LoadWidgetClassHelper(const TSoftClassPtr<UUserWidget>& SoftClassPtr)
{
    if (SoftClassPtr.IsNull())
        return nullptr;
    return SoftClassPtr.LoadSynchronous();
}

TSubclassOf<UUserWidget> FUIConfigData::GetLocalizedWidgetClass() const
{
    UxyLanguageManager* LangMgr = UxyLanguageManager::GetLanguageManager();
    if (LangMgr)
    {
        ELanguageType CurrentLang = LangMgr->GetCurrentLanguage();
        if (const TSoftClassPtr<UUserWidget>* LocalizedClass = LocalizedWidgetClasses.Find(CurrentLang))
        {
            if (!LocalizedClass->IsNull())
                return LoadWidgetClassHelper(*LocalizedClass);
        }
    }
    // 回退到默认 WidgetClass
    return LoadWidgetClassHelper(WidgetClass);
}

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