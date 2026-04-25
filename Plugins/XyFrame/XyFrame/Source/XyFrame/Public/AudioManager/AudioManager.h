// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SingletonBase/SingletonBase.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "AudioManager.generated.h"

class UAudioEffectController;

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

USTRUCT(BlueprintType)
struct FAudioConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FName SoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudioCategory Category = EAudioCategory::SFX;

    // 原有单个资源（保留兼容）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    // 新增：随机池（多个资源）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<TSoftObjectPtr<USoundBase>> SoundAssets;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0"))
    float FadeInTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fade", meta = (ClampMin = "0.0"))
    float FadeOutTime = 0.0f;

    // [NEW] 是否允许重新播放：当该音频正在播放时，再次触发播放会停止旧的并重新开始
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Behavior")
    bool bAllowRestart = false;
};

UENUM(BlueprintType)
enum class EAudioEffectType : uint8
{
    None,
    LowPassFilter,
    HighPassFilter,
    BandPassFilter,
};

USTRUCT(BlueprintType)
struct FAudioEffectPreset : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    EAudioEffectType EffectType = EAudioEffectType::LowPassFilter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowPass", meta = (EditCondition = "EffectType == EAudioEffectType::LowPassFilter"))
    float InitialCutoffFrequency = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect|LowPass", meta = (EditCondition = "EffectType == EAudioEffectType::LowPassFilter"))
    float InitialResonance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
    bool bAutoApply = false;
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

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void Initialize(UDataTable* InAudioDataTable);

    // ------------------------------------------------------------
    // 基础播放 (新增概率播放重载)
    // ------------------------------------------------------------
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

    // 带概率的播放 (Probability: 0~1)
    UFUNCTION(BlueprintCallable, Category = "Audio", meta = (WorldContext = "WorldContextObject"))
    UAudioComponent* PlaySoundWithProbability(
        UObject* WorldContextObject,
        FName SoundID,
        float Probability,
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

    // ------------------------------------------------------------
    // SFX
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFX(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, FVector Location = FVector::ZeroVector, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFXWithProbability(UObject* WorldContextObject, FName SoundID, float Probability, AActor* AttachActor = nullptr, FVector Location = FVector::ZeroVector, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopSFX(FName SoundID, float FadeOutTime = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopAllSFX(float FadeOutTime = 0.0f);

    // ------------------------------------------------------------
    // BGM
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void StopBGM(float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PauseBGM();

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void ResumeBGM();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    UAudioComponent* GetCurBGMAudioComponent() { return CurrentBGMComponent; }

    // ------------------------------------------------------------
    // Ambient
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayAmbient(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAmbient(FName SoundID, float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAllAmbient(float FadeTime = -1.0f);

    // ------------------------------------------------------------
    // Voice
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoice(
        UObject* WorldContextObject,
        FName SoundID,
        AActor* AttachActor = nullptr,
        float FadeInTime = -1.0f,
        float FadeOutTime = -1.0f,
        float PitchMultiplier = 1.0f
    );

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceWithProbability(
        UObject* WorldContextObject,
        FName SoundID,
        float Probability,
        AActor* AttachActor = nullptr,
        float FadeInTime = -1.0f,
        float FadeOutTime = -1.0f,
        float PitchMultiplier = 1.0f
    );

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopVoice(FName SoundID, float FadeOutTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopAllVoice(float FadeOutTime = -1.0f);

    // 暂停/恢复 Voice
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PauseVoice();

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void ResumeVoice();

    // ------------------------------------------------------------
    // UI
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void PlayUISound(UObject* WorldContextObject, FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void StopUISound(FName SoundID, float FadeOutTime = 0.0f);

    // ------------------------------------------------------------
    // 通用暂停/恢复
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void PauseSound(FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void ResumeSound(FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void PauseAllSoundsByCategory(EAudioCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void ResumeAllSoundsByCategory(EAudioCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void PauseAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio|Control")
    void ResumeAllSounds();

    // ------------------------------------------------------------
    // Volume
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetCategoryVolume(EAudioCategory Category, float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    float GetCategoryVolume(EAudioCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetAllVolumes(float BGMVolume, float SFXVolume, float AmbientVolume, float VoiceVolume, float UIVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void ResetAllVolumes();

    // ------------------------------------------------------------
    // Query
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    bool IsSoundPlaying(FName SoundID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCount() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCountByCategory(EAudioCategory Category) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    bool IsVoicePlaying() const;

    // ------------------------------------------------------------
    // Debug
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintAudioSystemStatus();

    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintCategoryStatus(EAudioCategory Category);

    // ------------------------------------------------------------
    // Shutdown
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void Shutdown();

    // ------------------------------------------------------------
    // Delegates
    // ------------------------------------------------------------
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundStarted OnSoundStarted;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundFinished OnSoundFinished;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnCategoryVolumeChanged OnCategoryVolumeChanged;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsInitialized() const { return AudioDataTable != nullptr; }

    // ------------------------------------------------------------
    // Effect system
    // ------------------------------------------------------------
    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void InitializeEffectSystem(UDataTable* InEffectPresetTable);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    UAudioEffectController* GetOrCreateEffectController(UAudioComponent* AudioComponent);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void ApplyEffectPresetBySoundID(FName SoundID, FName EffectPresetRowName);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void SetSoundLowPassCutoff(FName SoundID, float Cutoff, float Resonance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void StartSoundLowPassLerp(FName SoundID, float TargetCutoff, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void ClearSoundEffect(FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Effect")
    void ShutdownEffectSystem();

protected:
    void TickEffectControllers();

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

    TArray<FTimerHandle> PendingDestroyTimers;

    // Effect system
    UPROPERTY()
    TObjectPtr<UDataTable> EffectPresetTable;

    UPROPERTY()
    TMap<TObjectPtr<UAudioComponent>, TObjectPtr<UAudioEffectController>> ComponentEffectControllers;

    FTimerHandle EffectTickTimerHandle;

    // 资源缓存
    TMap<FSoftObjectPath, TObjectPtr<USoundBase>> LoadedSoundCache;

    const FAudioConfig* GetAudioConfig(FName SoundID) const;
    USoundBase* LoadSoundAsset(const TSoftObjectPtr<USoundBase>& SoftPtr);
    USoundBase* GetRandomSoundFromConfig(const FAudioConfig* Config);

    // 概率辅助函数
    bool ShouldPlayByProbability(float Probability) const;

    // [NEW] 立即停止相同 SoundID 的所有实例（用于重新播放）
    void StopAllInstancesOfSoundID(FName SoundID);

    UFUNCTION()
    void HandleAudioFinished();

    UWorld* GetWorld() const;

    void FadeOutAndDestroyAudioComponent(UAudioComponent* AudioComponent, float FadeOutTime);
    void SafelyDestroyAudioComponent(UAudioComponent* AudioComponent);
};