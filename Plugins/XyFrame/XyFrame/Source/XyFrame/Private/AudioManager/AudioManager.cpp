// Fill out your copyright notice in the Page Settings.
#include "AudioManager/AudioManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

template<>
UAudioManager* TSingleton<UAudioManager>::SingletonInstance = nullptr;

UAudioManager::UAudioManager()
    : AudioDataTable(nullptr)
    , CurrentBGMComponent(nullptr)
    , CurrentVoiceComponent(nullptr)
{
}

UAudioManager::~UAudioManager()
{
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

    CategoryVolumes.Add(EAudioCategory::BGM, 0.8f);
    CategoryVolumes.Add(EAudioCategory::SFX, 0.8f);
    CategoryVolumes.Add(EAudioCategory::Ambient, 0.8f);
    CategoryVolumes.Add(EAudioCategory::Voice, 0.8f);
    CategoryVolumes.Add(EAudioCategory::UI, 0.8f);
}

void UAudioManager::Initialize(UDataTable* InAudioDataTable)
{
    if (AudioDataTable == InAudioDataTable)
    {
        UE_LOG(LogTemp, Log, TEXT("AudioManager already initialized with same data table, skipping."));
        return;
    }
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

    // 淡入时间：若传入值<=0且配置中有正数，则使用配置值
    float ActualFadeIn = FadeInTime;
    if (ActualFadeIn <= 0.0f && Config->FadeInTime > 0.0f)
        ActualFadeIn = Config->FadeInTime;

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;
    USoundBase* SoundAsset = Config->SoundAsset.LoadSynchronous();
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load sound: %s"), *SoundID.ToString());
        return nullptr;
    }

    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(AttachActor ? AttachActor : World->GetWorldSettings());
    if (!AudioComponent) return nullptr;

    AudioComponent->SetSound(SoundAsset);
    float FinalPitchMultiplier = PitchMultiplier * Config->PitchMultiplier;
    AudioComponent->SetPitchMultiplier(FinalPitchMultiplier);
    float VolumeMultiplier = Config->DefaultVolume * CategoryVolumes[Config->Category];
    AudioComponent->SetVolumeMultiplier(VolumeMultiplier);

    bool bAllowSpatialization = true;
    switch (Config->Category)
    {
    case EAudioCategory::BGM:
    case EAudioCategory::UI:
        bAllowSpatialization = false;
        break;
    default:
        bAllowSpatialization = true;
        break;
    }
    AudioComponent->bAllowSpatialization = bAllowSpatialization;
    AudioComponent->AttenuationSettings = Config->AttenuationSettings.Get();

    if (AttachActor && AttachActor->GetRootComponent())
    {
        AudioComponent->AttachToComponent(AttachActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        AudioComponent->SetWorldLocation(Location);
    }

    AudioComponent->RegisterComponent();
    AudioComponent->OnAudioFinished.AddDynamic(this, &UAudioManager::HandleAudioFinished);
    ActiveComponents.Add(AudioComponent, SoundID);

    if (ActualFadeIn > 0.0f)
        AudioComponent->FadeIn(ActualFadeIn, VolumeMultiplier);
    else
        AudioComponent->Play();

    OnSoundStarted.Broadcast(SoundID);
    UE_LOG(LogTemp, Log, TEXT("Playing sound: %s, Category: %s, FadeIn: %.2f"), *SoundID.ToString(), *UEnum::GetValueAsString(Config->Category), ActualFadeIn);
    return AudioComponent;
}

void UAudioManager::HandleAudioFinished()
{
    TArray<UAudioComponent*> CompletedComponents;
    for (auto& Pair : ActiveComponents)
    {
        UAudioComponent* Comp = Pair.Key;
        if (IsValid(Comp) && !Comp->IsPlaying())
            CompletedComponents.Add(Comp);
    }

    for (UAudioComponent* Comp : CompletedComponents)
    {
        const FName SoundID = ActiveComponents[Comp];
        OnSoundFinished.Broadcast(SoundID);
        ActiveComponents.Remove(Comp);
        if (Comp == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
        if (Comp == CurrentBGMComponent) CurrentBGMComponent = nullptr;
        if (Comp->IsValidLowLevel()) Comp->DestroyComponent();
    }
}

void UAudioManager::StopSound(FName SoundID, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID) ComponentsToStop.Add(Pair.Key);
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        float ActualFadeOut = FadeOutTime;
        if (ActualFadeOut <= 0.0f)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f)
                ActualFadeOut = Config->FadeOutTime;
        }

        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
            if (Component == CurrentBGMComponent) CurrentBGMComponent = nullptr;
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllSounds(float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    ActiveComponents.GenerateKeyArray(ComponentsToStop);

    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (FadeOutTime > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, FadeOutTime);
        else
        {
            if (Component && Component->IsValidLowLevel())
            {
                Component->Stop();
                Component->DestroyComponent();
            }
        }
    }
    ActiveComponents.Empty();
    CurrentBGMComponent = nullptr;
    CurrentVoiceComponent = nullptr;
}

void UAudioManager::StopAllSoundsByCategory(EAudioCategory Category, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == Category) ComponentsToStop.Add(Pair.Key);
    }

    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (FadeOutTime > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, FadeOutTime);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component == CurrentVoiceComponent && Category == EAudioCategory::Voice) CurrentVoiceComponent = nullptr;
            if (Component == CurrentBGMComponent && Category == EAudioCategory::BGM) CurrentBGMComponent = nullptr;
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
}

// ========== SFX ==========
void UAudioManager::PlaySFX(UObject* WorldContextObject, FName SoundID, AActor* AttachActor, FVector Location, float PitchMultiplier)
{
    PlaySound(WorldContextObject, SoundID, AttachActor, Location, 0.0f, 0.0f, PitchMultiplier);
}

void UAudioManager::StopSFX(FName SoundID, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::SFX) ComponentsToStop.Add(Pair.Key);
        }
    }
    for (UAudioComponent* Component : ComponentsToStop)
    {
        float ActualFadeOut = FadeOutTime;
        if (ActualFadeOut <= 0.0f)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f) ActualFadeOut = Config->FadeOutTime;
        }
        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllSFX(float FadeOutTime) { StopAllSoundsByCategory(EAudioCategory::SFX, FadeOutTime); }

// ========== BGM ==========
void UAudioManager::PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime)
{
    if (CurrentBGMComponent && CurrentBGMComponent->IsPlaying())
        StopBGM(FadeTime);

    float ActualFadeIn = FadeTime;
    if (ActualFadeIn < 0.0f)
    {
        const FAudioConfig* Config = GetAudioConfig(SoundID);
        if (Config && Config->FadeInTime > 0.0f)
            ActualFadeIn = Config->FadeInTime;
        else
            ActualFadeIn = 0.0f;
    }
    CurrentBGMComponent = PlaySound(WorldContextObject, SoundID, nullptr, FVector::ZeroVector, ActualFadeIn);
}

void UAudioManager::StopBGM(float FadeTime)
{
    if (CurrentBGMComponent)
    {
        float ActualFadeOut = FadeTime;
        if (ActualFadeOut < 0.0f)
        {
            FName SoundID = ActiveComponents.FindRef(CurrentBGMComponent);
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f)
                ActualFadeOut = Config->FadeOutTime;
            else
                ActualFadeOut = 0.0f;
        }

        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(CurrentBGMComponent, ActualFadeOut);
        else
        {
            CurrentBGMComponent->Stop();
            ActiveComponents.Remove(CurrentBGMComponent);
            if (CurrentBGMComponent->IsValidLowLevel()) CurrentBGMComponent->DestroyComponent();
        }
        CurrentBGMComponent = nullptr;
    }
}

void UAudioManager::PauseBGM() { if (CurrentBGMComponent) CurrentBGMComponent->SetPaused(true); }
void UAudioManager::ResumeBGM() { if (CurrentBGMComponent) CurrentBGMComponent->SetPaused(false); }

// ========== Ambient ==========
void UAudioManager::PlayAmbient(UObject* WorldContextObject, FName SoundID, AActor* AttachActor, float FadeTime)
{
    float ActualFadeIn = FadeTime;
    if (ActualFadeIn < 0.0f)
    {
        const FAudioConfig* Config = GetAudioConfig(SoundID);
        if (Config && Config->FadeInTime > 0.0f) ActualFadeIn = Config->FadeInTime;
        else ActualFadeIn = 0.0f;
    }
    PlaySound(WorldContextObject, SoundID, AttachActor, FVector::ZeroVector, ActualFadeIn);
}

void UAudioManager::StopAmbient(FName SoundID, float FadeTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::Ambient) ComponentsToStop.Add(Pair.Key);
        }
    }
    for (UAudioComponent* Component : ComponentsToStop)
    {
        float ActualFadeOut = FadeTime;
        if (ActualFadeOut < 0.0f)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f) ActualFadeOut = Config->FadeOutTime;
            else ActualFadeOut = 0.0f;
        }
        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllAmbient(float FadeTime) { StopAllSoundsByCategory(EAudioCategory::Ambient, FadeTime); }

// ========== Voice 核心修改：停止旧语音，而非暂停 ==========
void UAudioManager::PlayVoice(
    UObject* WorldContextObject,
    FName SoundID,
    AActor* AttachActor,
    float FadeInTime,
    float FadeOutTime,
    float PitchMultiplier)
{
    // 1. 如果有正在播放的 Voice，先停止它（支持淡出）
    if (CurrentVoiceComponent && CurrentVoiceComponent->IsPlaying())
    {
        FName OldSoundID = ActiveComponents.FindRef(CurrentVoiceComponent);
        if (OldSoundID != NAME_None)
        {
            // 确定停止旧语音的淡出时间
            float ActualFadeOut = FadeOutTime;
            if (ActualFadeOut < 0.0f)
            {
                const FAudioConfig* OldConfig = GetAudioConfig(OldSoundID);
                if (OldConfig && OldConfig->FadeOutTime > 0.0f)
                    ActualFadeOut = OldConfig->FadeOutTime;
                else
                    ActualFadeOut = 0.0f;
            }
            StopVoice(OldSoundID, ActualFadeOut);
        }
        else
        {
            // 无法获取 SoundID，直接停止组件
            if (FadeOutTime > 0.0f)
                FadeOutAndDestroyAudioComponent(CurrentVoiceComponent, FadeOutTime);
            else
            {
                CurrentVoiceComponent->Stop();
                ActiveComponents.Remove(CurrentVoiceComponent);
                if (CurrentVoiceComponent->IsValidLowLevel()) CurrentVoiceComponent->DestroyComponent();
            }
            CurrentVoiceComponent = nullptr;
        }
    }

    // 2. 确定新语音的淡入时间
    float ActualFadeIn = FadeInTime;
    if (ActualFadeIn < 0.0f)
    {
        const FAudioConfig* NewConfig = GetAudioConfig(SoundID);
        if (NewConfig && NewConfig->FadeInTime > 0.0f)
            ActualFadeIn = NewConfig->FadeInTime;
        else
            ActualFadeIn = 0.0f;
    }

    // 3. 播放新语音
    UAudioComponent* NewVoiceComponent = PlaySound(WorldContextObject, SoundID, AttachActor, FVector::ZeroVector, ActualFadeIn, 0.0f, PitchMultiplier);
    if (NewVoiceComponent)
        CurrentVoiceComponent = NewVoiceComponent;
}

void UAudioManager::StopVoice(FName SoundID, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::Voice) ComponentsToStop.Add(Pair.Key);
        }
    }
    for (UAudioComponent* Component : ComponentsToStop)
    {
        float ActualFadeOut = FadeOutTime;
        if (ActualFadeOut < 0.0f)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f) ActualFadeOut = Config->FadeOutTime;
            else ActualFadeOut = 0.0f;
        }
        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
        if (Component == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
    }
}

void UAudioManager::StopAllVoice(float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == EAudioCategory::Voice) ComponentsToStop.Add(Pair.Key);
    }
    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (FadeOutTime > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, FadeOutTime);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
    CurrentVoiceComponent = nullptr;
}

// 注意：由于我们改为直接停止旧语音，不再需要 PauseCurrentVoice 和 ResumeCurrentVoice
// 如果你仍需要恢复功能，可以保留，但按新逻辑已无暂停的语音。

// ========== UI ==========
void UAudioManager::PlayUISound(UObject* WorldContextObject, FName SoundID)
{
    PlaySound(WorldContextObject, SoundID, nullptr, FVector::ZeroVector, 0.0f, 0.0f, 1.0f);
}

void UAudioManager::StopUISound(FName SoundID, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->Category == EAudioCategory::UI) ComponentsToStop.Add(Pair.Key);
        }
    }
    for (UAudioComponent* Component : ComponentsToStop)
    {
        float ActualFadeOut = FadeOutTime;
        if (ActualFadeOut <= 0.0f)
        {
            const FAudioConfig* Config = GetAudioConfig(SoundID);
            if (Config && Config->FadeOutTime > 0.0f) ActualFadeOut = Config->FadeOutTime;
        }
        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
        {
            Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component->IsValidLowLevel()) Component->DestroyComponent();
        }
    }
}

// ========== Volume ==========
void UAudioManager::SetCategoryVolume(EAudioCategory Category, float NewVolume)
{
    float ClampedVolume = FMath::Clamp(NewVolume, 0.0f, 1.0f);
    CategoryVolumes.Add(Category, ClampedVolume);
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

void UAudioManager::ResetAllVolumes() { SetAllVolumes(0.8f, 0.8f, 0.8f, 0.8f, 0.8f); }

// ========== Query ==========
bool UAudioManager::IsSoundPlaying(FName SoundID) const
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID && Pair.Key && Pair.Key->IsPlaying()) return true;
    }
    return false;
}

int32 UAudioManager::GetActiveSoundCount() const { return ActiveComponents.Num(); }

int32 UAudioManager::GetActiveSoundCountByCategory(EAudioCategory Category) const
{
    int32 Count = 0;
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config && Config->Category == Category) Count++;
    }
    return Count;
}

bool UAudioManager::IsVoicePlaying() const
{
    return CurrentVoiceComponent != nullptr && CurrentVoiceComponent->IsPlaying();
}

// ========== Debug ==========
void UAudioManager::PrintAudioSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== Audio System Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Active Sounds: %d"), ActiveComponents.Num());
    TArray<EAudioCategory> AllCategories = { EAudioCategory::BGM, EAudioCategory::SFX, EAudioCategory::Ambient, EAudioCategory::Voice, EAudioCategory::UI };
    TMap<EAudioCategory, int32> CategoryCounts;
    for (EAudioCategory Category : AllCategories) CategoryCounts.Add(Category, 0);
    for (auto& Pair : ActiveComponents)
    {
        const FAudioConfig* Config = GetAudioConfig(Pair.Value);
        if (Config) CategoryCounts[Config->Category]++;
    }
    for (EAudioCategory Category : AllCategories)
    {
        UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *UEnum::GetValueAsString(Category), CategoryCounts[Category]);
    }
    UE_LOG(LogTemp, Log, TEXT("Current BGM: %s"), CurrentBGMComponent ? TEXT("Playing") : TEXT("None"));
    UE_LOG(LogTemp, Log, TEXT("Current Voice: %s"), CurrentVoiceComponent ? TEXT("Playing") : TEXT("None"));
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

// ========== Internal ==========
UWorld* UAudioManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE) return Context.World();
        }
    }
    return nullptr;
}

void UAudioManager::FadeOutAndDestroyAudioComponent(UAudioComponent* AudioComponent, float FadeTime)
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
                    if (AudioComponent == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
                    if (AudioComponent == CurrentBGMComponent) CurrentBGMComponent = nullptr;
                    AudioComponent->DestroyComponent();
                }
            }, FadeTime, false);
    }
}