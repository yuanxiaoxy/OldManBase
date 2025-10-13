// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "SaveGameDataTypes.generated.h"

// 玩家存档数据结构
USTRUCT(BlueprintType)
struct FPlayerSaveData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    int32 Experience = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    FString CharacterName = TEXT("Player");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    TArray<FString> InventoryItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    TMap<FString, int32> InventoryQuantities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    TMap<FString, float> PlayerStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Player")
    TMap<FString, bool> UnlockedAbilities;

    // 构造函数
    FPlayerSaveData()
        : Health(100.0f)
        , MaxHealth(100.0f)
        , Experience(0)
        , Level(1)
    {
    }

    // 便捷方法 - 移除了 UFUNCTION
    bool IsAlive() const { return Health > 0.0f; }

    float GetHealthPercentage() const { return MaxHealth > 0 ? Health / MaxHealth : 0.0f; }

    void AddExperience(int32 ExpAmount)
    {
        Experience += ExpAmount;
        // 简单的升级逻辑
        if (Experience >= Level * 100)
        {
            Level++;
            MaxHealth += 10.0f;
            Health = MaxHealth;
        }
    }

    void AddItem(const FString& ItemID, int32 Quantity = 1)
    {
        if (InventoryQuantities.Contains(ItemID))
        {
            InventoryQuantities[ItemID] += Quantity;
        }
        else
        {
            InventoryItems.Add(ItemID);
            InventoryQuantities.Add(ItemID, Quantity);
        }
    }

    bool RemoveItem(const FString& ItemID, int32 Quantity = 1)
    {
        if (!InventoryQuantities.Contains(ItemID)) return false;

        int32& CurrentQuantity = InventoryQuantities[ItemID];
        if (CurrentQuantity <= Quantity)
        {
            InventoryQuantities.Remove(ItemID);
            InventoryItems.Remove(ItemID);
            return true;
        }
        else
        {
            CurrentQuantity -= Quantity;
            return true;
        }
    }

    int32 GetItemQuantity(const FString& ItemID) const
    {
        return InventoryQuantities.Contains(ItemID) ? InventoryQuantities[ItemID] : 0;
    }

    bool HasItem(const FString& ItemID) const
    {
        return InventoryQuantities.Contains(ItemID) && InventoryQuantities[ItemID] > 0;
    }

    void SetStat(const FString& StatName, float Value)
    {
        PlayerStats.Add(StatName, Value);
    }

    float GetStat(const FString& StatName, float DefaultValue = 0.0f) const
    {
        return PlayerStats.Contains(StatName) ? PlayerStats[StatName] : DefaultValue;
    }

    void UnlockAbility(const FString& AbilityID)
    {
        UnlockedAbilities.Add(AbilityID, true);
    }

    bool IsAbilityUnlocked(const FString& AbilityID) const
    {
        return UnlockedAbilities.Contains(AbilityID) && UnlockedAbilities[AbilityID];
    }

    // 序列化支持
    bool Serialize(FArchive& Ar)
    {
        Ar << Location;
        Ar << Rotation;
        Ar << Health;
        Ar << MaxHealth;
        Ar << Experience;
        Ar << Level;
        Ar << CharacterName;
        Ar << InventoryItems;
        Ar << InventoryQuantities;
        Ar << PlayerStats;
        Ar << UnlockedAbilities;
        return true;
    }
};

// 世界存档数据结构
USTRUCT(BlueprintType)
struct FWorldSaveData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, bool> ActivatedTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, FVector> MovedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, bool> DestroyedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, FTransform> ActorTransforms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, FString> WorldStateData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, bool> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, int32> QuestProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|World")
    TMap<FString, bool> DiscoveredLocations;

    // 便捷方法 - 移除了 UFUNCTION
    void SetActorTransform(const FString& ActorID, const FTransform& Transform)
    {
        ActorTransforms.Add(ActorID, Transform);
    }

    FTransform GetActorTransform(const FString& ActorID, const FTransform& DefaultTransform = FTransform::Identity) const
    {
        return ActorTransforms.Contains(ActorID) ? ActorTransforms[ActorID] : DefaultTransform;
    }

    void ActivateTrigger(const FString& TriggerID)
    {
        ActivatedTriggers.Add(TriggerID, true);
    }

    bool IsTriggerActivated(const FString& TriggerID) const
    {
        return ActivatedTriggers.Contains(TriggerID) && ActivatedTriggers[TriggerID];
    }

    void MarkActorDestroyed(const FString& ActorID)
    {
        DestroyedActors.Add(ActorID, true);
    }

    bool IsActorDestroyed(const FString& ActorID) const
    {
        return DestroyedActors.Contains(ActorID) && DestroyedActors[ActorID];
    }

    void CompleteQuest(const FString& QuestID)
    {
        CompletedQuests.Add(QuestID, true);
    }

    bool IsQuestCompleted(const FString& QuestID) const
    {
        return CompletedQuests.Contains(QuestID) && CompletedQuests[QuestID];
    }

    void SetQuestProgress(const FString& QuestID, int32 Progress)
    {
        QuestProgress.Add(QuestID, Progress);
    }

    int32 GetQuestProgress(const FString& QuestID) const
    {
        return QuestProgress.Contains(QuestID) ? QuestProgress[QuestID] : 0;
    }

    void DiscoverLocation(const FString& LocationID)
    {
        DiscoveredLocations.Add(LocationID, true);
    }

    bool IsLocationDiscovered(const FString& LocationID) const
    {
        return DiscoveredLocations.Contains(LocationID) && DiscoveredLocations[LocationID];
    }

    void SetWorldState(const FString& Key, const FString& Value)
    {
        WorldStateData.Add(Key, Value);
    }

    FString GetWorldState(const FString& Key, const FString& DefaultValue = TEXT("")) const
    {
        return WorldStateData.Contains(Key) ? WorldStateData[Key] : DefaultValue;
    }

    // 序列化支持
    bool Serialize(FArchive& Ar)
    {
        Ar << ActivatedTriggers;
        Ar << MovedActors;
        Ar << DestroyedActors;
        Ar << ActorTransforms;
        Ar << WorldStateData;
        Ar << CompletedQuests;
        Ar << QuestProgress;
        Ar << DiscoveredLocations;
        return true;
    }
};

// 设置数据结构
USTRUCT(BlueprintType)
struct FSettingsData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    float MusicVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    float SFXVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    float VoiceVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    FString Language = TEXT("en");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    int32 GraphicsQuality = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    bool bFullscreen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    FIntPoint Resolution = FIntPoint(1920, 1080);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    float MouseSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    bool bInvertYAxis = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    bool bShowSubtitles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    bool bShowTutorials = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Settings")
    int32 DifficultyLevel = 1;

    // 便捷方法 - 移除了 UFUNCTION
    void SetVolumeLevels(float NewMaster, float NewMusic, float NewSFX, float NewVoice = 0.8f)
    {
        MasterVolume = FMath::Clamp(NewMaster, 0.0f, 1.0f);
        MusicVolume = FMath::Clamp(NewMusic, 0.0f, 1.0f);
        SFXVolume = FMath::Clamp(NewSFX, 0.0f, 1.0f);
        VoiceVolume = FMath::Clamp(NewVoice, 0.0f, 1.0f);
    }

    void SetResolution(int32 Width, int32 Height)
    {
        Resolution = FIntPoint(Width, Height);
    }

    FString GetResolutionString() const
    {
        return FString::Printf(TEXT("%dx%d"), Resolution.X, Resolution.Y);
    }

    void SetGraphicsQuality(int32 Quality)
    {
        GraphicsQuality = FMath::Clamp(Quality, 0, 3);
    }

    void SetDifficulty(int32 Difficulty)
    {
        DifficultyLevel = FMath::Clamp(Difficulty, 0, 2);
    }

    // 序列化支持
    bool Serialize(FArchive& Ar)
    {
        Ar << MasterVolume;
        Ar << MusicVolume;
        Ar << SFXVolume;
        Ar << VoiceVolume;
        Ar << Language;
        Ar << GraphicsQuality;
        Ar << bFullscreen;
        Ar << Resolution;
        Ar << MouseSensitivity;
        Ar << bInvertYAxis;
        Ar << bShowSubtitles;
        Ar << bShowTutorials;
        Ar << DifficultyLevel;
        return true;
    }
};

// 进度数据结构
USTRUCT(BlueprintType)
struct FProgressData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TArray<FString> CompletedLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TMap<FString, bool> UnlockedAchievements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TMap<FString, int32> LevelScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TMap<FString, float> LevelCompletionTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    int32 TotalPlayTimeSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    int32 TotalDeaths = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    int32 TotalKills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TMap<FString, int32> CollectiblesFound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Progress")
    TMap<FString, FDateTime> AchievementUnlockTimes;

    // 便捷方法 - 移除了 UFUNCTION
    void CompleteLevel(const FString& LevelName, int32 Score = 0, float CompletionTime = 0.0f)
    {
        if (!CompletedLevels.Contains(LevelName))
        {
            CompletedLevels.Add(LevelName);
        }

        if (Score > 0)
        {
            LevelScores.Add(LevelName, Score);
        }

        if (CompletionTime > 0)
        {
            LevelCompletionTimes.Add(LevelName, CompletionTime);
        }
    }

    bool IsLevelCompleted(const FString& LevelName) const
    {
        return CompletedLevels.Contains(LevelName);
    }

    void UnlockAchievement(const FString& AchievementID)
    {
        UnlockedAchievements.Add(AchievementID, true);
        AchievementUnlockTimes.Add(AchievementID, FDateTime::Now());
    }

    bool IsAchievementUnlocked(const FString& AchievementID) const
    {
        return UnlockedAchievements.Contains(AchievementID) && UnlockedAchievements[AchievementID];
    }

    FDateTime GetAchievementUnlockTime(const FString& AchievementID) const
    {
        return AchievementUnlockTimes.Contains(AchievementID) ? AchievementUnlockTimes[AchievementID] : FDateTime::MinValue();
    }

    void AddCollectible(const FString& CollectibleType, int32 Count = 1)
    {
        if (CollectiblesFound.Contains(CollectibleType))
        {
            CollectiblesFound[CollectibleType] += Count;
        }
        else
        {
            CollectiblesFound.Add(CollectibleType, Count);
        }
    }

    int32 GetCollectibleCount(const FString& CollectibleType) const
    {
        return CollectiblesFound.Contains(CollectibleType) ? CollectiblesFound[CollectibleType] : 0;
    }

    void AddPlayTime(int32 Seconds)
    {
        TotalPlayTimeSeconds += Seconds;
    }

    FString GetFormattedPlayTime() const
    {
        int32 Hours = TotalPlayTimeSeconds / 3600;
        int32 Minutes = (TotalPlayTimeSeconds % 3600) / 60;
        int32 Seconds = TotalPlayTimeSeconds % 60;
        return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Seconds);
    }

    void AddDeath()
    {
        TotalDeaths++;
    }

    void AddKill(int32 Count = 1)
    {
        TotalKills += Count;
    }

    float GetKillDeathRatio() const
    {
        return TotalDeaths > 0 ? static_cast<float>(TotalKills) / TotalDeaths : TotalKills;
    }

    int32 GetCompletedLevelCount() const
    {
        return CompletedLevels.Num();
    }

    int32 GetUnlockedAchievementCount() const
    {
        int32 Count = 0;
        for (const auto& Pair : UnlockedAchievements)
        {
            if (Pair.Value)
            {
                Count++;
            }
        }
        return Count;
    }

    // 序列化支持
    bool Serialize(FArchive& Ar)
    {
        Ar << CompletedLevels;
        Ar << UnlockedAchievements;
        Ar << LevelScores;
        Ar << LevelCompletionTimes;
        Ar << TotalPlayTimeSeconds;
        Ar << TotalDeaths;
        Ar << TotalKills;
        Ar << CollectiblesFound;
        Ar << AchievementUnlockTimes;
        return true;
    }
};

// 自定义数据容器 - 用于存储任意类型的游戏数据
USTRUCT(BlueprintType)
struct FCustomSaveData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, FString> StringData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, float> FloatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, int32> IntData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, bool> BoolData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, FVector> VectorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, FRotator> RotatorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveGame|Custom")
    TMap<FString, FTransform> TransformData;

    // 便捷方法 - 移除了 UFUNCTION
    void SetString(const FString& Key, const FString& Value)
    {
        StringData.Add(Key, Value);
    }

    FString GetString(const FString& Key, const FString& DefaultValue = TEXT("")) const
    {
        return StringData.Contains(Key) ? StringData[Key] : DefaultValue;
    }

    void SetFloat(const FString& Key, float Value)
    {
        FloatData.Add(Key, Value);
    }

    float GetFloat(const FString& Key, float DefaultValue = 0.0f) const
    {
        return FloatData.Contains(Key) ? FloatData[Key] : DefaultValue;
    }

    void SetInt(const FString& Key, int32 Value)
    {
        IntData.Add(Key, Value);
    }

    int32 GetInt(const FString& Key, int32 DefaultValue = 0) const
    {
        return IntData.Contains(Key) ? IntData[Key] : DefaultValue;
    }

    void SetBool(const FString& Key, bool Value)
    {
        BoolData.Add(Key, Value);
    }

    bool GetBool(const FString& Key, bool DefaultValue = false) const
    {
        return BoolData.Contains(Key) ? BoolData[Key] : DefaultValue;
    }

    void SetVector(const FString& Key, const FVector& Value)
    {
        VectorData.Add(Key, Value);
    }

    FVector GetVector(const FString& Key, const FVector& DefaultValue = FVector::ZeroVector) const
    {
        return VectorData.Contains(Key) ? VectorData[Key] : DefaultValue;
    }

    void SetRotator(const FString& Key, const FRotator& Value)
    {
        RotatorData.Add(Key, Value);
    }

    FRotator GetRotator(const FString& Key, const FRotator& DefaultValue = FRotator::ZeroRotator) const
    {
        return RotatorData.Contains(Key) ? RotatorData[Key] : DefaultValue;
    }

    void SetTransform(const FString& Key, const FTransform& Value)
    {
        TransformData.Add(Key, Value);
    }

    FTransform GetTransform(const FString& Key, const FTransform& DefaultValue = FTransform::Identity) const
    {
        return TransformData.Contains(Key) ? TransformData[Key] : DefaultValue;
    }

    void RemoveKey(const FString& Key)
    {
        StringData.Remove(Key);
        FloatData.Remove(Key);
        IntData.Remove(Key);
        BoolData.Remove(Key);
        VectorData.Remove(Key);
        RotatorData.Remove(Key);
        TransformData.Remove(Key);
    }

    void ClearAll()
    {
        StringData.Empty();
        FloatData.Empty();
        IntData.Empty();
        BoolData.Empty();
        VectorData.Empty();
        RotatorData.Empty();
        TransformData.Empty();
    }

    // 序列化支持
    bool Serialize(FArchive& Ar)
    {
        Ar << StringData;
        Ar << FloatData;
        Ar << IntData;
        Ar << BoolData;
        Ar << VectorData;
        Ar << RotatorData;
        Ar << TransformData;
        return true;
    }
};