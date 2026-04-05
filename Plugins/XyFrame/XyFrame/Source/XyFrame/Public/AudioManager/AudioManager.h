// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "AudioManager.generated.h"

// 音频类别
UENUM(BlueprintType)
enum class EAudioCategory : uint8
{
    BGM      UMETA(DisplayName = "Background Music"),
    SFX      UMETA(DisplayName = "Sound Effects"),
    Ambient  UMETA(DisplayName = "Ambient Sound"),
    Voice    UMETA(DisplayName = "Voice"),
    UI       UMETA(DisplayName = "UI Sounds")
};

ENUM_RANGE_BY_COUNT(EAudioCategory, 5)

// 音频配置结构
USTRUCT(BlueprintType)
struct FAudioConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudioCategory Category = EAudioCategory::SFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DefaultVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundAttenuation> AttenuationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    int32 Priority = 0;

    // 淡入时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0"))
    float FadeInTime = 0.0f;

    // 淡出时间（秒）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0"))
    float FadeOutTime = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoundStarted, FName, SoundID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoundFinished, FName, SoundID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCategoryVolumeChanged, EAudioCategory, Category, float, NewVolume);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UAudioManager : public USingletonBase
{
    GENERATED_BODY()

    DECLARE_SINGLETON(UAudioManager)

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeAudioManager();

    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio", meta = (DisplayName = "Get Audio Manager"))
    static UAudioManager* GetAudioManager() { return GetInstance(); }

    UAudioManager();
    virtual ~UAudioManager() override;

    // ========== 基础音频接口 ==========
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void Initialize(UDataTable* InAudioDataTable);

    UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
    UAudioComponent* PlaySound(
        UObject* WorldContextObject,
        FName SoundID,
        AActor* AttachActor = nullptr,
        FVector Location = FVector::ZeroVector,
        float FadeInTime = 0.0f,
        float Delay = 0.0f,
        float PitchMultiplier = 1.0f
    );

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(FName SoundID, float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds(float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSoundsByCategory(EAudioCategory Category, float FadeOutTime = 0.0f);

    // ========== 分类音频接口 ==========
    // SFX
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFX(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, FVector Location = FVector::ZeroVector, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopSFX(FName SoundID, float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopAllSFX(float FadeOutTime = 0.0f);

    // BGM
    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void StopBGM(float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PauseBGM();

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void ResumeBGM();

    // Ambient
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayAmbient(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAmbient(FName SoundID, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAllAmbient(float FadeTime = -1.0f);

    // Voice
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoice(
        UObject* WorldContextObject,
        FName SoundID,
        AActor* AttachActor = nullptr,
        float FadeInTime = -1.0f,      // -1 使用新音频配置中的淡入时间
        float FadeOutTime = -1.0f,     // -1 使用旧音频配置中的淡出时间（停止旧语音时）
        float PitchMultiplier = 1.0f
    );

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopVoice(FName SoundID, float FadeOutTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopAllVoice(float FadeOutTime = -1.0f);

    // UI
    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void PlayUISound(UObject* WorldContextObject, FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void StopUISound(FName SoundID, float FadeOutTime = 0.0f);

    // ========== 音量控制 ==========
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetCategoryVolume(EAudioCategory Category, float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    float GetCategoryVolume(EAudioCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetAllVolumes(float BGMVolume, float SFXVolume, float AmbientVolume, float VoiceVolume, float UIVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void ResetAllVolumes();

    // ========== 查询 ==========
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    bool IsSoundPlaying(FName SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCount() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCountByCategory(EAudioCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    bool IsVoicePlaying() const;

    // ========== 调试 ==========
    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintAudioSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintCategoryStatus(EAudioCategory Category);

    // ========== 委托 ==========
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundStarted OnSoundStarted;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundFinished OnSoundFinished;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnCategoryVolumeChanged OnCategoryVolumeChanged;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsInitialized() const { return AudioDataTable != nullptr; }

private:
    UPROPERTY()
    UDataTable* AudioDataTable;

    UPROPERTY()
    TMap<UAudioComponent*, FName> ActiveComponents;

    TMap<EAudioCategory, float> CategoryVolumes;

    UPROPERTY()
    UAudioComponent* CurrentBGMComponent;

    UPROPERTY()
    UAudioComponent* CurrentVoiceComponent;

    const FAudioConfig* GetAudioConfig(FName SoundID) const;

    UFUNCTION()
    void HandleAudioFinished();

    UWorld* GetWorld() const;

    void FadeOutAndDestroyAudioComponent(UAudioComponent* AudioComponent, float FadeOutTime);
};