#include "LanguageManager/xyLanguageManager.h"
#include "Kismet/GameplayStatics.h"

const FString UxyLanguageManager::SaveSlotName = TEXT("LanguageSetting");
const int32 UxyLanguageManager::SaveUserIndex = 0;

template<>
UxyLanguageManager* TSingleton<UxyLanguageManager>::SingletonInstance = nullptr;

void UxyLanguageManager::InitializeLanguageManager()
{
    LoadLanguageSetting();
    UE_LOG(LogTemp, Log, TEXT("LanguageManager Initialized, Current Language: %d"), (int32)CurrentLanguage);
}

void UxyLanguageManager::InitializeSingleton()
{
    InitializeLanguageManager();
}

void UxyLanguageManager::SetCurrentLanguage(ELanguageType NewLanguage, bool bSaveToLocal)
{
    if (CurrentLanguage == NewLanguage)
        return;

    CurrentLanguage = NewLanguage;
    if (bSaveToLocal)
    {
        bool bSaved = SaveLanguageSetting();
        if (!bSaved)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to save language setting to disk!"));
        }
    }

    OnLanguageChanged.Broadcast(CurrentLanguage);
    UE_LOG(LogTemp, Log, TEXT("Language changed to %d"), (int32)CurrentLanguage);
}

FString UxyLanguageManager::GetLanguageCode(ELanguageType Language) const
{
    switch (Language)
    {
    case ELanguageType::Chinese: return TEXT("zh");
    case ELanguageType::English: return TEXT("en");
    default: return TEXT("zh");
    }
}

bool UxyLanguageManager::SaveLanguageSetting()
{
    ULanguageSaveGame* SaveGame = Cast<ULanguageSaveGame>(UGameplayStatics::CreateSaveGameObject(ULanguageSaveGame::StaticClass()));
    if (!SaveGame)
    {
        UE_LOG(LogTemp, Error, TEXT("SaveLanguageSetting: Failed to create save game object!"));
        return false;
    }

    SaveGame->SavedLanguage = CurrentLanguage;

    bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, SaveUserIndex);
    if (bSaved)
    {
        UE_LOG(LogTemp, Log, TEXT("SaveLanguageSetting: Successfully saved language (%d) to slot '%s'"), (int32)CurrentLanguage, *SaveSlotName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("SaveLanguageSetting: Failed to save game to slot '%s' (UserIndex=%d)"), *SaveSlotName, SaveUserIndex);
    }
    return bSaved;
}

void UxyLanguageManager::LoadLanguageSetting()
{
    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
    {
        ULanguageSaveGame* LoadedGame = Cast<ULanguageSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
        if (LoadedGame)
        {
            CurrentLanguage = LoadedGame->SavedLanguage;
            UE_LOG(LogTemp, Log, TEXT("LoadLanguageSetting: Loaded language %d from slot '%s'"), (int32)CurrentLanguage, *SaveSlotName);
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("LoadLanguageSetting: Failed to cast loaded save game. Using default."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("LoadLanguageSetting: No save game found. Using default Chinese."));
    }

    CurrentLanguage = ELanguageType::Chinese;
}