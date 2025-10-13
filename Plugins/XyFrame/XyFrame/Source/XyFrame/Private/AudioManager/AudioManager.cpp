// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioManager/AudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// 静态实例定义
template<>
UAudioManager* TSingleton<UAudioManager>::SingletonInstance = nullptr;

UAudioManager::UAudioManager()
    : AudioDataTable(nullptr)
    , CurrentBGMComponent(nullptr)
{
}

UAudioManager::~UAudioManager()
{
    // 清理所有音频组件
    StopAllSounds();
}

void UAudioManager::InitializeSingleton()
{
    UE_LOG(LogTemp, Log, TEXT("AudioManager InitializeSingleton called"));
    InitializeAudioManager();
}

void UAudioManager::InitializeAudioManager()
{
    UE_LOG(LogTemp, Log, TEXT("Audio Manager Initialized"));

    // 初始化默认音量
    CategoryVolumes.Add(EAudioCategory::BGM, 0.8f);
    CategoryVolumes.Add(EAudioCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudioCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudioCategory::Voice, 0.8f);
    CategoryVolumes.Add(EAudioCategory::UI, 0.8f);
}

void UAudioManager::Initialize(UDataTable* InAudioDataTable)
{
    AudioDataTable = InAudioDataTable;
    UE_LOG(LogTemp, Log, TEXT("AudioManager initialized with data table"));
}

const FAudioConfig* UAudioManager::GetAudioConfig(FName SoundID) const
{
    if (!AudioDataTable) return nullptr;

    static const FString ContextString(TEXT("AudioManager Context"));
    return AudioDataTable->FindRow<FAudioConfig>(SoundID, ContextString);
}

UAudioComponent* UAudioManager::PlaySound(
    UObject* WorldContextObject,
    FName SoundID,
    AActor* AttachActor,
    FVector Location,
    float FadeInTime,
    float Delay,
    float PitchMultiplier)
{
    if (Delay > 0.0f)
    {
        // 延迟播放处理
        FTimerDelegate TimerDel;
        FTimerHandle TimerHandle;

        UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            TimerDel.BindLambda([=, this]()
                {
                    PlaySound(WorldContextObject, SoundID, AttachActor, Location, FadeInTime, 0.0f, PitchMultiplier);
                });

            World->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
        }
        return nullptr;
    }

    if (!WorldContextObject) return nullptr;

    const FAudioConfig* Config = GetAudioConfig(SoundID);
    if (!Config || Config->SoundAsset.IsNull())
    {
        UE_LOG(LogTemp, Error, TEXT("SoundID %s not found or invalid!"), *SoundID.ToString());
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;

    USoundBase* SoundAsset = Config->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load sound: %s"), *SoundID.ToString());
        return nullptr;
    }

    // 创建音频组件
    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(AttachActor ? AttachActor : World->GetWorldSettings());
    if (!AudioComponent) return nullptr;

    // 配置音频组件
    AudioComponent->SetSound(SoundAsset);

    // 设置音调
    float FinalPitchMultiplier = PitchMultiplier * Config->PitchMultiplier;
    AudioComponent->SetPitchMultiplier(FinalPitchMultiplier);

    // 设置音量
    float VolumeMultiplier = Config->DefaultVolume * CategoryVolumes[Config->Category];
    AudioComponent->SetVolumeMultiplier(VolumeMultiplier);

    // 空间化设置
    bool bAllowSpatialization = true;
    switch (Config->Category)
    {
    case EAudioCategory::BGM:
    case EAudioCategory::UI:
        bAllowSpatialization = false;
        break;
    case EAudioCategory::SFX:
    case EAudioCategory::Ambient:
    case EAudioCategory::Voice:
        bAllowSpatialization = true;
        break;
    }

    AudioComponent->bAllowSpatialization = bAllowSpatialization;
    AudioComponent->AttenuationSettings = Config->AttenuationSettings.Get();

    if (AttachActor && AttachActor->GetRootComponent())
    {
        AudioComponent->AttachToComponent(
            AttachActor->GetRootComponent(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );
    }
    else
    {
        AudioComponent->SetWorldLocation(Location);
    }

    // 注册组件并开始播放
    AudioComponent->RegisterComponent();
    AudioComponent->OnAudioFinished.AddDynamic(this, &UAudioManager::HandleAudioFinished);

    // 记录活跃组件
    ActiveComponents.Add(AudioComponent, SoundID);

    // 淡入或直接播放
    if (FadeInTime > 0.0f)
    {
        AudioComponent->FadeIn(FadeInTime, VolumeMultiplier);
    }
    else
    {
        AudioComponent->Play();
    }

    // 触发事件
    OnSoundStarted.Broadcast(SoundID);

    UE_LOG(LogTemp, Log, TEXT("Playing sound: %s, Category: %s"),
        *SoundID.ToString(),
        *UEnum::GetValueAsString(Config->Category));

    return AudioComponent;
}

void UAudioManager::HandleAudioFinished()
{
    // 清理完成的音频组件
    TArray<UAudioComponent*> CompletedComponents;

    for (auto& Pair : ActiveComponents)
    {
        UAudioComponent* Comp = Pair.Key;
        if (IsValid(Comp) && !Comp->IsPlaying())
        {
            CompletedComponents.Add(Comp);
        }
    }

    for (UAudioComponent* Comp : CompletedComponents)
    {
        const FName SoundID = ActiveComponents[Comp];
        OnSoundFinished.Broadcast(SoundID);
        ActiveComponents.Remove(Comp);

        if (Comp->IsValidLowLevel())
        {
            Comp->DestroyComponent();
        }
    }
}

void UAudioManager::StopSound(FName SoundID)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            ComponentsToStop.Add(Pair.Key);
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        Component->Stop();
        ActiveComponents.Remove(Component);

        if (Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllSounds()
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Key && Pair.Key->IsValidLowLevel())
        {
            Pair.Key->Stop();
            Pair.Key->DestroyComponent();
        }
    }
    ActiveComponents.Empty();

    // 同时清空当前BGM引用
    CurrentBGMComponent = nullptr;
}

void UAudioManager::StopAllSoundsByCategory(EAudioCategory Category)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == Category)
        {
            ComponentsToStop.Add(Pair.Key);
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        Component->Stop();
        ActiveComponents.Remove(Component);

        if (Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }

    // 如果是BGM类别，还需要清空当前BGM引用
    if (Category == EAudioCategory::BGM)
    {
        CurrentBGMComponent = nullptr;
    }
}

// ========== SFX 音效管理实现 ==========

void UAudioManager::PlaySFX(UObject* WorldContextObject, FName SoundID, AActor* AttachActor, FVector Location, float PitchMultiplier)
{
    PlaySound(
        WorldContextObject,
        SoundID,
        AttachActor,
        Location,
        0.0f,  // 无淡入
        0.0f,  // 无延迟
        PitchMultiplier
    );
}

void UAudioManager::StopSFX(FName SoundID)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::SFX)
            {
                ComponentsToStop.Add(Pair.Key);
            }
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        Component->Stop();
        ActiveComponents.Remove(Component);
        if (Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllSFX()
{
    StopAllSoundsByCategory(EAudioCategory::SFX);
}

// ========== BGM 管理实现 ==========

void UAudioManager::PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime)
{
    if (CurrentBGMComponent && CurrentBGMComponent->IsPlaying())
    {
        StopBGM(FadeTime);
    }

    CurrentBGMComponent = PlaySound(
        WorldContextObject,
        SoundID,
        nullptr,
        FVector::ZeroVector,
        FadeTime
    );
}

void UAudioManager::StopBGM(float FadeTime)
{
    if (CurrentBGMComponent)
    {
        if (FadeTime > 0.0f)
        {
            FadeOutAudioComponent(CurrentBGMComponent, FadeTime);
        }
        else
        {
            CurrentBGMComponent->Stop();
            ActiveComponents.Remove(CurrentBGMComponent);
            if (CurrentBGMComponent->IsValidLowLevel())
            {
                CurrentBGMComponent->DestroyComponent();
            }
        }
        CurrentBGMComponent = nullptr;
    }
}

void UAudioManager::PauseBGM()
{
    if (CurrentBGMComponent)
    {
        CurrentBGMComponent->SetPaused(true);
    }
}

void UAudioManager::ResumeBGM()
{
    if (CurrentBGMComponent)
    {
        CurrentBGMComponent->SetPaused(false);
    }
}

// ========== 环境音管理实现 ==========

void UAudioManager::PlayAmbient(UObject* WorldContextObject, FName SoundID, AActor* AttachActor, float FadeTime)
{
    PlaySound(
        WorldContextObject,
        SoundID,
        AttachActor,
        FVector::ZeroVector,
        FadeTime
    );
}

void UAudioManager::StopAmbient(FName SoundID, float FadeTime)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::Ambient)
            {
                ComponentsToStop.Add(Pair.Key);
            }
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (FadeTime > 0.0f)
        {
            FadeOutAudioComponent(Component, FadeTime);
        }
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel())
            {
                Component->DestroyComponent();
            }
        }
    }
}

void UAudioManager::StopAllAmbient(float FadeTime)
{
    StopAllSoundsByCategory(EAudioCategory::Ambient);
}

// ========== 语音管理实现 ==========

void UAudioManager::PlayVoice(UObject* WorldContextObject, FName SoundID, AActor* AttachActor)
{
    PlaySound(
        WorldContextObject,
        SoundID,
        AttachActor
    );
}

void UAudioManager::StopVoice(FName SoundID)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::Voice)
            {
                ComponentsToStop.Add(Pair.Key);
            }
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        Component->Stop();
        ActiveComponents.Remove(Component);
        if (Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllVoice()
{
    StopAllSoundsByCategory(EAudioCategory::Voice);
}

// ========== UI音效管理实现 ==========

void UAudioManager::PlayUISound(UObject* WorldContextObject, FName SoundID)
{
    PlaySound(
        WorldContextObject,
        SoundID,
        nullptr,
        FVector::ZeroVector,
        0.0f,
        0.0f,
        1.0f
    );
}

void UAudioManager::StopUISound(FName SoundID)
{
    TArray<UAudioComponent*> ComponentsToStop;

    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::UI)
            {
                ComponentsToStop.Add(Pair.Key);
            }
        }
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        Component->Stop();
        ActiveComponents.Remove(Component);
        if (Component->IsValidLowLevel())
        {
            Component->DestroyComponent();
        }
    }
}

// ========== 音量控制实现 ==========

void UAudioManager::SetCategoryVolume(EAudioCategory Category, float NewVolume)
{
    float ClampedVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    CategoryVolumes.Add(Category, ClampedVolume);

    // 更新所有活跃音频的音量
    for (auto& Pair : ActiveComponents)
    {
        UAudioComponent* Component = Pair.Key;
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);

        if (Component && Config && Config->Category == Category)
        {
            Component->SetVolumeMultiplier(Config->DefaultVolume * ClampedVolume);
        }
    }

    OnCategoryVolumeChanged.Broadcast(Category, ClampedVolume);
}

float UAudioManager::GetCategoryVolume(EAudioCategory Category) const
{
    const float* Volume = CategoryVolumes.Find(Category);
    return Volume ? *Volume : 0.8f;
}

void UAudioManager::SetAllVolumes(float BGMVolume, float SFXVolume, float AmbientVolume, float VoiceVolume, float UIVolume)
{
    SetCategoryVolume(EAudioCategory::BGM, BGMVolume);
    SetCategoryVolume(EAudioCategory::SFX, SFXVolume);
    SetCategoryVolume(EAudioCategory::Ambient, AmbientVolume);
    SetCategoryVolume(EAudioCategory::Voice, VoiceVolume);
    SetCategoryVolume(EAudioCategory::UI, UIVolume);
}

void UAudioManager::ResetAllVolumes()
{
    SetAllVolumes(0.8f, 0.8f, 0.8f, 0.8f, 0.8f);
}

// ========== 音频状态查询实现 ==========

bool UAudioManager::IsSoundPlaying(FName SoundID) const
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID && Pair.Key && Pair.Key->IsPlaying())
        {
            return true;
        }
    }
    return false;
}

int32 UAudioManager::GetActiveSoundCount() const
{
    return ActiveComponents.Num();
}

int32 UAudioManager::GetActiveSoundCountByCategory(EAudioCategory Category) const
{
    int32 Count = 0;
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == Category)
        {
            Count++;
        }
    }
    return Count;
}

// ========== 调试工具实现 ==========

void UAudioManager::PrintAudioSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== Audio System Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Active Sounds: %d"), ActiveComponents.Num());

    // 按类别统计
    TMap<EAudioCategory, int32> CategoryCounts;

    // 手动定义所有音频类别进行迭代
    TArray<EAudioCategory> AllCategories = {
        EAudioCategory::BGM,
        EAudioCategory::SFX,
        EAudioCategory::Ambient,
        EAudioCategory::Voice,
        EAudioCategory::UI
    };

    for (EAudioCategory Category : AllCategories)
    {
        CategoryCounts.Add(Category, 0);
    }

    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config)
        {
            CategoryCounts[Config->Category]++;
        }
    }

    for (EAudioCategory Category : AllCategories)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *UEnum::GetValueAsString(Category), CategoryCounts[Category]);
    }

    UE_LOG(LogTemp, Log, TEXT("Current BGM: %s"), CurrentBGMComponent ? TEXT("Playing") : TEXT("None"));
    UE_LOG(LogTemp, Log, TEXT("=== End Status ==="));
}

void UAudioManager::PrintCategoryStatus(EAudioCategory Category)
{
    UE_LOG(LogTemp, Log, TEXT("=== %s Audio Status ==="), *UEnum::GetValueAsString(Category));

    int32 Count = 0;
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == Category)
        {
            UE_LOG(LogTemp, Log, TEXT("  - %s"), *Pair.Value.ToString());
            Count++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Total: %d sounds"), Count);
    UE_LOG(LogTemp, Log, TEXT("=== End %s Status ==="), *UEnum::GetValueAsString(Category));
}

// ========== 内部辅助方法 ==========

UWorld* UAudioManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
            {
                return Context.World();
            }
        }
    }
    return nullptr;
}

void UAudioManager::FadeOutAudioComponent(UAudioComponent* AudioComponent, float FadeTime)
{
    if (!AudioComponent) return;

    AudioComponent->FadeOut(FadeTime, 0.0f);

    FTimerHandle TimerHandle;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, [this, AudioComponent]()
            {
                if (AudioComponent && AudioComponent->IsValidLowLevel())
                {
                    ActiveComponents.Remove(AudioComponent);
                    AudioComponent->DestroyComponent();
                }
            }, FadeTime, false);
    }
}