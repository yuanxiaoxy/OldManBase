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

// 添加枚举范围定义
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
};

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoundStarted, FName, SoundID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoundFinished, FName, SoundID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCategoryVolumeChanged, EAudioCategory, Category, float, NewVolume);

UCLASS(Blueprintable, BlueprintType)
class XYFRAME_API UAudioManager : public USingletonBase
{
    GENERATED_BODY()

    // 单例声明
    DECLARE_SINGLETON(UAudioManager)

public:
    // 初始化音频管理器
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeAudioManager();

    // 重写单例初始化方法
    virtual void InitializeSingleton() override;
    virtual void DestroyCurSingleton() override { DestroyInstance(); }

    // 获取管理器实例的蓝图可调用方法
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio", meta = (DisplayName = "Get Audio Manager"))
    static UAudioManager* GetAudioManager() { return GetInstance(); }

    // 构造函数
    UAudioManager();
    virtual ~UAudioManager() override;

    // ========== 基础音频接口 ==========

    // 初始化音频系统
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void Initialize(UDataTable* InAudioDataTable);

    // 播放音频（通用接口）
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

    // 停止特定SoundID的所有音频
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(FName SoundID);

    // 停止所有音频
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSounds();

    // 停止特定类别的所有音频
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllSoundsByCategory(EAudioCategory Category);

    // ========== 分类音频接口 ==========

    // SFX 管理
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlaySFX(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, FVector Location = FVector::ZeroVector, float PitchMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopSFX(FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void StopAllSFX();

    // BGM 管理
    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void StopBGM(float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PauseBGM();

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void ResumeBGM();

    // 环境音管理
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void PlayAmbient(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr, float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAmbient(FName SoundID, float FadeTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void StopAllAmbient(float FadeTime = 1.0f);

    // 语音管理
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoice(UObject* WorldContextObject, FName SoundID, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopVoice(FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void StopAllVoice();

    // UI音效管理
    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void PlayUISound(UObject* WorldContextObject, FName SoundID);

    UFUNCTION(BlueprintCallable, Category = "Audio|UI")
    void StopUISound(FName SoundID);

    // ========== 音量控制 ==========

    // 设置类别音量
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetCategoryVolume(EAudioCategory Category, float NewVolume);

    // 获取类别音量
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    float GetCategoryVolume(EAudioCategory Category) const;

    // 设置所有音量
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetAllVolumes(float BGMVolume, float SFXVolume, float AmbientVolume, float VoiceVolume, float UIVolume);

    // 重置所有音量到默认值
    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void ResetAllVolumes();

    // ========== 音频状态查询 ==========

    // 检查音频是否正在播放
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    bool IsSoundPlaying(FName SoundID) const;

    // 获取活跃音频数量
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCount() const;

    // 获取特定类别的活跃音频数量
    UFUNCTION(BlueprintCallable, Category = "Audio|Query")
    int32 GetActiveSoundCountByCategory(EAudioCategory Category) const;

    // ========== 调试工具 ==========

    // 打印音频系统状态
    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintAudioSystemStatus();

    // 打印特定类别的音频状态
    UFUNCTION(BlueprintCallable, Category = "Audio|Debug")
    void PrintCategoryStatus(EAudioCategory Category);

    // ========== 委托 ==========

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundStarted OnSoundStarted;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnSoundFinished OnSoundFinished;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FOnCategoryVolumeChanged OnCategoryVolumeChanged;

private:
    // 音频数据表
    UPROPERTY()
    UDataTable* AudioDataTable;

    // 活跃音频组件映射
    UPROPERTY()
    TMap<UAudioComponent*, FName> ActiveComponents;

    // 类别音量
    TMap<EAudioCategory, float> CategoryVolumes;

    // 当前BGM组件
    UPROPERTY()
    UAudioComponent* CurrentBGMComponent;

    // 内部方法
    const FAudioConfig* GetAudioConfig(FName SoundID) const;

    // 音频完成处理
    UFUNCTION()
    void HandleAudioFinished();

    // 获取World的辅助方法
    UWorld* GetWorld() const;

    // 淡出音频组件
    void FadeOutAudioComponent(UAudioComponent* AudioComponent, float FadeTime);
};