// Fill out your copyright notice in the Page Settings.
#include "AudioManager/AudioManager.h"
#include "AudioManager/AudioEffectController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundAttenuation.h"

template<>
UAudioManager* TSingleton<UAudioManager>::SingletonInstance = nullptr;

UAudioManager::UAudioManager()
    : AudioDataTable(nullptr)
    , CurrentBGMComponent(nullptr)
    , CurrentVoiceComponent(nullptr)
    , EffectPresetTable(nullptr)
{
}

UAudioManager::~UAudioManager()
{
    Shutdown();
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

USoundBase* UAudioManager::LoadSoundAsset(const TSoftObjectPtr<USoundBase>& SoftPtr)
{
    if (SoftPtr.IsNull())
        return nullptr;

    FSoftObjectPath AssetPath = SoftPtr.ToSoftObjectPath();
    if (TObjectPtr<USoundBase>* Cached = LoadedSoundCache.Find(AssetPath))
    {
        if (IsValid(*Cached))
            return *Cached;
        else
            LoadedSoundCache.Remove(AssetPath);
    }

    USoundBase* Loaded = SoftPtr.LoadSynchronous();
    if (IsValid(Loaded))
    {
        LoadedSoundCache.Add(AssetPath, Loaded);
    }
    return Loaded;
}

USoundBase* UAudioManager::GetRandomSoundFromConfig(const FAudioConfig* Config)
{
    if (!Config)
        return nullptr;

    if (Config->SoundAssets.Num() > 0)
    {
        TArray<TSoftObjectPtr<USoundBase>> ValidAssets;
        for (const TSoftObjectPtr<USoundBase>& SoftPtr : Config->SoundAssets)
        {
            if (!SoftPtr.IsNull())
                ValidAssets.Add(SoftPtr);
        }
        if (ValidAssets.Num() > 0)
        {
            int32 Index = FMath::RandRange(0, ValidAssets.Num() - 1);
            return LoadSoundAsset(ValidAssets[Index]);
        }
    }

    if (!Config->SoundAsset.IsNull())
        return LoadSoundAsset(Config->SoundAsset);

    return nullptr;
}

bool UAudioManager::ShouldPlayByProbability(float Probability) const
{
    Probability = FMath::Clamp(Probability, 0.0f, 1.0f);
    return FMath::FRand() <= Probability;
}

UAudioComponent* UAudioManager::FindAndReuseExistingComponent(const FAudioConfig* Config, AActor* AttachActor, const FVector& Location)
{
    if (!Config) return nullptr;

    UAudioComponent* ExistingComp = nullptr;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == Config->SoundID)
        {
            ExistingComp = Pair.Key;
            break;
        }
    }

    if (!IsValid(ExistingComp))
        return nullptr;

    ExistingComp->Stop();
    ExistingComp->SetPaused(false);

    if (TObjectPtr<UAudioEffectController>* Ctrl = ComponentEffectControllers.Find(ExistingComp))
    {
        (*Ctrl)->ClearEffect();
        ComponentEffectControllers.Remove(ExistingComp);
    }

    USoundBase* NewSound = GetRandomSoundFromConfig(Config);
    if (!IsValid(NewSound))
        return nullptr;
    ExistingComp->SetSound(NewSound);

    bool bAllowSpatial = (Config->Category != EAudioCategory::BGM && Config->Category != EAudioCategory::UI);
    ExistingComp->bAllowSpatialization = bAllowSpatial;

    if (!Config->AttenuationSettings.IsNull())
    {
        USoundAttenuation* Attenuation = Cast<USoundAttenuation>(Config->AttenuationSettings.LoadSynchronous());
        if (IsValid(Attenuation))
            ExistingComp->AttenuationSettings = Attenuation;
        else
            ExistingComp->AttenuationSettings = nullptr;
    }
    else
    {
        ExistingComp->AttenuationSettings = nullptr;
    }

    if (IsValid(AttachActor) && AttachActor->GetRootComponent())
    {
        ExistingComp->AttachToComponent(AttachActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    }
    else
    {
        ExistingComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        ExistingComp->SetWorldLocation(Location);
    }

    return ExistingComp;
}

UAudioComponent* UAudioManager::PlaySoundWithProbability(
    UObject* WorldContextObject,
    FName SoundID,
    float Probability,
    AActor* AttachActor,
    FVector Location,
    float FadeInTime,
    float Delay,
    float PitchMultiplier)
{
    if (ShouldPlayByProbability(Probability))
    {
        return PlaySound(WorldContextObject, SoundID, AttachActor, Location, FadeInTime, Delay, PitchMultiplier);
    }
    else
    {
        UE_LOG(LogTemp, Verbose, TEXT("PlaySoundWithProbability: SoundID %s skipped (Probability=%.2f)"), *SoundID.ToString(), Probability);
        return nullptr;
    }
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
    const FAudioConfig* Config = GetAudioConfig(SoundID);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("SoundID %s not found in data table!"), *SoundID.ToString());
        return nullptr;
    }

    // 重新播放复用组件
    if (Config->bAllowRestart)
    {
        UAudioComponent* ReusedComp = FindAndReuseExistingComponent(Config, AttachActor, Location);
        if (ReusedComp)
        {
            UE_LOG(LogTemp, Log, TEXT("PlaySound: Reusing existing component for SoundID %s (bAllowRestart=true)"), *SoundID.ToString());

            ReusedComp->SetPitchMultiplier(PitchMultiplier * Config->PitchMultiplier);
            float VolumeMultiplier = Config->DefaultVolume * CategoryVolumes[Config->Category];
            ReusedComp->SetVolumeMultiplier(VolumeMultiplier);

            float ActualFadeIn = FadeInTime;
            if (ActualFadeIn <= 0.0f && Config->FadeInTime > 0.0f)
                ActualFadeIn = Config->FadeInTime;

            ReusedComp->OnAudioFinished.RemoveDynamic(this, &UAudioManager::HandleAudioFinished);
            ReusedComp->OnAudioFinished.AddDynamic(this, &UAudioManager::HandleAudioFinished);

            if (ActualFadeIn > 0.0f)
                ReusedComp->FadeIn(ActualFadeIn, VolumeMultiplier);
            else
                ReusedComp->Play();

            OnSoundStarted.Broadcast(SoundID);
            return ReusedComp;
        }
    }

    // 延迟播放
    if (Delay > 0.0f)
    {
        FTimerDelegate TimerDel;
        FTimerHandle TimerHandle;
        UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            TWeakObjectPtr<UAudioManager> WeakThis(this);
            TimerDel.BindLambda([WeakThis, WorldContextObject, SoundID, AttachActor, Location, FadeInTime, PitchMultiplier]()
                {
                    if (WeakThis.IsValid())
                        WeakThis->PlaySound(WorldContextObject, SoundID, AttachActor, Location, FadeInTime, 0.0f, PitchMultiplier);
                });
            World->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
            PendingDestroyTimers.Add(TimerHandle);
        }
        return nullptr;
    }

    // 加载资源
    USoundBase* SoundAsset = GetRandomSoundFromConfig(Config);
    if (!IsValid(SoundAsset))
    {
        UE_LOG(LogTemp, Error, TEXT("No valid sound asset for SoundID %s"), *SoundID.ToString());
        return nullptr;
    }

    if (Config->bLooping)
    {
        if (USoundWave* SoundWave = Cast<USoundWave>(SoundAsset))
        {
            SoundWave->bLooping = true;
        }
        else if (USoundCue* SoundCue = Cast<USoundCue>(SoundAsset))
        {
            UE_LOG(LogTemp, Warning, TEXT("Looping requested for SoundID '%s' but asset is a SoundCue."), *SoundID.ToString());
        }
    }

    // 自动停止同类声音（Voice/BGM）
    if (Config->Category == EAudioCategory::Voice || Config->Category == EAudioCategory::BGM)
    {
        TArray<UAudioComponent*> ComponentsToStop;
        for (auto& Pair : ActiveComponents)
        {
            const FAudioConfig* OtherConfig = GetAudioConfig(Pair.Value);
            if (OtherConfig && OtherConfig->Category == Config->Category)
                ComponentsToStop.Add(Pair.Key);
        }
        for (UAudioComponent* Comp : ComponentsToStop)
        {
            if (!IsValid(Comp)) continue;
            FName OldSoundID = ActiveComponents.FindRef(Comp);
            const FAudioConfig* OldConfig = GetAudioConfig(OldSoundID);
            float FadeOut = OldConfig ? OldConfig->FadeOutTime : 0.0f;
            if (FadeOut > 0.0f)
                FadeOutAndDestroyAudioComponent(Comp, FadeOut);
            else
                SafelyDestroyAudioComponent(Comp);
        }
    }

    float ActualFadeIn = FadeInTime;
    if (ActualFadeIn <= 0.0f && Config->FadeInTime > 0.0f)
        ActualFadeIn = Config->FadeInTime;

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;

    UObject* Outer = IsValid(AttachActor) ? AttachActor : World->GetWorldSettings();
    if (!IsValid(Outer)) Outer = World->GetWorldSettings();

    UAudioComponent* AudioComponent = NewObject<UAudioComponent>(Outer);
    if (!AudioComponent) return nullptr;

    AudioComponent->SetSound(SoundAsset);
    AudioComponent->SetPitchMultiplier(PitchMultiplier * Config->PitchMultiplier);
    float VolumeMultiplier = Config->DefaultVolume * CategoryVolumes[Config->Category];
    AudioComponent->SetVolumeMultiplier(VolumeMultiplier);

    bool bAllowSpatialization = (Config->Category != EAudioCategory::BGM && Config->Category != EAudioCategory::UI);
    AudioComponent->bAllowSpatialization = bAllowSpatialization;

    if (!Config->AttenuationSettings.IsNull())
    {
        USoundAttenuation* Attenuation = Cast<USoundAttenuation>(Config->AttenuationSettings.LoadSynchronous());
        if (IsValid(Attenuation))
            AudioComponent->AttenuationSettings = Attenuation;
        else
            AudioComponent->AttenuationSettings = nullptr;
    }
    else
    {
        AudioComponent->AttenuationSettings = nullptr;
    }

    AudioComponent->bAutoDestroy = false;

    if (IsValid(AttachActor) && AttachActor->GetRootComponent())
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
    UE_LOG(LogTemp, Log, TEXT("Playing sound: %s, Category: %s, FadeIn: %.2f, Looping: %d"), *SoundID.ToString(), *UEnum::GetValueAsString(Config->Category), ActualFadeIn, Config->bLooping);
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
        if (TObjectPtr<UAudioEffectController>* Ctrl = ComponentEffectControllers.Find(Comp))
        {
            (*Ctrl)->ClearEffect();
            ComponentEffectControllers.Remove(Comp);
        }

        const FName SoundID = ActiveComponents[Comp];
        OnSoundFinished.Broadcast(SoundID);
        ActiveComponents.Remove(Comp);
        if (Comp == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
        if (Comp == CurrentBGMComponent) CurrentBGMComponent = nullptr;
        if (IsValid(Comp)) Comp->DestroyComponent();
    }
}

void UAudioManager::StopSound(FName SoundID, float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    for (auto& Pair : ActiveComponents)
        if (Pair.Value == SoundID) ComponentsToStop.Add(Pair.Key);

    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (!IsValid(Component)) continue;
        float ActualFadeOut = FadeOutTime;
        if (ActualFadeOut <= 0.0f)
            if (const FAudioConfig* Config = GetAudioConfig(SoundID))
                ActualFadeOut = Config->FadeOutTime;

        if (ActualFadeOut > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, ActualFadeOut);
        else
            SafelyDestroyAudioComponent(Component);
    }
}

void UAudioManager::StopAllSounds(float FadeOutTime)
{
    TArray<UAudioComponent*> ComponentsToStop;
    ActiveComponents.GenerateKeyArray(ComponentsToStop);
    for (UAudioComponent* Component : ComponentsToStop)
    {
        if (!IsValid(Component)) continue;
        if (FadeOutTime > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, FadeOutTime);
        else
            SafelyDestroyAudioComponent(Component);
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
        if (!IsValid(Component)) continue;
        if (FadeOutTime > 0.0f)
            FadeOutAndDestroyAudioComponent(Component, FadeOutTime);
        else
            SafelyDestroyAudioComponent(Component);
    }
}

// ========== 分类快捷函数 ==========
void UAudioManager::PlaySFX(UObject* WC, FName ID, AActor* Attach, FVector Loc, float Pitch)
{
    PlaySound(WC, ID, Attach, Loc, 0.0f, 0.0f, Pitch);
}
void UAudioManager::PlaySFXWithProbability(UObject* WC, FName ID, float Prob, AActor* Attach, FVector Loc, float Pitch)
{
    PlaySoundWithProbability(WC, ID, Prob, Attach, Loc, 0.0f, 0.0f, Pitch);
}
void UAudioManager::StopSFX(FName ID, float FadeOut) { StopSound(ID, FadeOut); }
void UAudioManager::StopAllSFX(float FadeOut) { StopAllSoundsByCategory(EAudioCategory::SFX, FadeOut); }

void UAudioManager::PlayBGM(UObject* WC, FName ID, float FadeTime)
{
    float Fade = (FadeTime >= 0.0f) ? FadeTime : (GetAudioConfig(ID) ? GetAudioConfig(ID)->FadeInTime : 0.0f);
    PlaySound(WC, ID, nullptr, FVector::ZeroVector, Fade);
}
void UAudioManager::StopBGM(float FadeTime)
{
    if (IsValid(CurrentBGMComponent))
    {
        float Fade = (FadeTime >= 0.0f) ? FadeTime : 0.0f;
        if (Fade <= 0.0f)
            if (const FAudioConfig* Cfg = GetAudioConfig(ActiveComponents[CurrentBGMComponent]))
                Fade = Cfg->FadeOutTime;
        if (Fade > 0.0f)
            FadeOutAndDestroyAudioComponent(CurrentBGMComponent, Fade);
        else
            SafelyDestroyAudioComponent(CurrentBGMComponent);
        CurrentBGMComponent = nullptr;
    }
}
void UAudioManager::PauseBGM() { if (IsValid(CurrentBGMComponent)) CurrentBGMComponent->SetPaused(true); }
void UAudioManager::ResumeBGM() { if (IsValid(CurrentBGMComponent)) CurrentBGMComponent->SetPaused(false); }

void UAudioManager::PlayAmbient(UObject* WC, FName ID, AActor* Attach, float FadeTime)
{
    float Fade = (FadeTime >= 0.0f) ? FadeTime : (GetAudioConfig(ID) ? GetAudioConfig(ID)->FadeInTime : 0.0f);
    PlaySound(WC, ID, Attach, FVector::ZeroVector, Fade);
}
void UAudioManager::StopAmbient(FName ID, float FadeTime)
{
    TArray<UAudioComponent*> Comps;
    for (auto& Pair : ActiveComponents)
        if (Pair.Value == ID && GetAudioConfig(ID)->Category == EAudioCategory::Ambient)
            Comps.Add(Pair.Key);
    for (UAudioComponent* C : Comps)
    {
        if (!IsValid(C)) continue;
        float Fade = (FadeTime >= 0.0f) ? FadeTime : (GetAudioConfig(ID) ? GetAudioConfig(ID)->FadeOutTime : 0.0f);
        if (Fade > 0.0f) FadeOutAndDestroyAudioComponent(C, Fade);
        else SafelyDestroyAudioComponent(C);
    }
}
void UAudioManager::StopAllAmbient(float FadeTime) { StopAllSoundsByCategory(EAudioCategory::Ambient, FadeTime); }

void UAudioManager::PlayVoice(UObject* WC, FName ID, AActor* Attach, float FadeIn, float FadeOut, float Pitch)
{
    float Fade = (FadeIn >= 0.0f) ? FadeIn : (GetAudioConfig(ID) ? GetAudioConfig(ID)->FadeInTime : 0.0f);
    PlaySound(WC, ID, Attach, FVector::ZeroVector, Fade, 0.0f, Pitch);
}
void UAudioManager::PlayVoiceWithProbability(UObject* WC, FName ID, float Prob, AActor* Attach, float FadeIn, float FadeOut, float Pitch)
{
    if (ShouldPlayByProbability(Prob)) PlayVoice(WC, ID, Attach, FadeIn, FadeOut, Pitch);
}
void UAudioManager::StopVoice(FName ID, float FadeOut)
{
    TArray<UAudioComponent*> Comps;
    for (auto& Pair : ActiveComponents)
        if (Pair.Value == ID && GetAudioConfig(ID)->Category == EAudioCategory::Voice)
            Comps.Add(Pair.Key);
    for (UAudioComponent* C : Comps)
    {
        if (!IsValid(C)) continue;
        float Fade = (FadeOut >= 0.0f) ? FadeOut : (GetAudioConfig(ID) ? GetAudioConfig(ID)->FadeOutTime : 0.0f);
        if (Fade > 0.0f) FadeOutAndDestroyAudioComponent(C, Fade);
        else SafelyDestroyAudioComponent(C);
        if (C == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
    }
}
void UAudioManager::StopAllVoice(float FadeOut) { StopAllSoundsByCategory(EAudioCategory::Voice, FadeOut); }
void UAudioManager::PauseVoice() { if (IsValid(CurrentVoiceComponent)) CurrentVoiceComponent->SetPaused(true); }
void UAudioManager::ResumeVoice() { if (IsValid(CurrentVoiceComponent)) CurrentVoiceComponent->SetPaused(false); }

void UAudioManager::PlayUISound(UObject* WC, FName ID) { PlaySound(WC, ID, nullptr, FVector::ZeroVector, 0.0f); }
void UAudioManager::StopUISound(FName ID, float FadeOut) { StopSound(ID, FadeOut); }

// ========== 通用暂停/恢复 ==========
void UAudioManager::PauseSound(FName ID)
{
    for (auto& Pair : ActiveComponents) if (Pair.Value == ID && IsValid(Pair.Key)) Pair.Key->SetPaused(true);
}
void UAudioManager::ResumeSound(FName ID)
{
    for (auto& Pair : ActiveComponents) if (Pair.Value == ID && IsValid(Pair.Key)) Pair.Key->SetPaused(false);
}
void UAudioManager::PauseAllSoundsByCategory(EAudioCategory Cat)
{
    for (auto& Pair : ActiveComponents) if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value)) if (Cfg->Category == Cat && IsValid(Pair.Key)) Pair.Key->SetPaused(true);
}
void UAudioManager::ResumeAllSoundsByCategory(EAudioCategory Cat)
{
    for (auto& Pair : ActiveComponents) if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value)) if (Cfg->Category == Cat && IsValid(Pair.Key)) Pair.Key->SetPaused(false);
}
void UAudioManager::PauseAllSounds()
{
    for (auto& Pair : ActiveComponents) if (IsValid(Pair.Key)) Pair.Key->SetPaused(true);
}
void UAudioManager::ResumeAllSounds()
{
    for (auto& Pair : ActiveComponents) if (IsValid(Pair.Key)) Pair.Key->SetPaused(false);
}

// ========== 音量 ==========
void UAudioManager::SetCategoryVolume(EAudioCategory Cat, float Vol)
{
    float Clamped = FMath::Clamp(Vol, 0.0f, 1.0f);
    CategoryVolumes.Add(Cat, Clamped);
    for (auto& Pair : ActiveComponents)
        if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value))
            if (Cfg->Category == Cat && IsValid(Pair.Key))
                Pair.Key->SetVolumeMultiplier(Cfg->DefaultVolume * Clamped);
    OnCategoryVolumeChanged.Broadcast(Cat, Clamped);
}
float UAudioManager::GetCategoryVolume(EAudioCategory Cat) const { return CategoryVolumes.FindRef(Cat); }
void UAudioManager::SetAllVolumes(float BGM, float SFX, float Amb, float Voice, float UI)
{
    SetCategoryVolume(EAudioCategory::BGM, BGM); SetCategoryVolume(EAudioCategory::SFX, SFX); SetCategoryVolume(EAudioCategory::Ambient, Amb); SetCategoryVolume(EAudioCategory::Voice, Voice); SetCategoryVolume(EAudioCategory::UI, UI);
}
void UAudioManager::ResetAllVolumes() { SetAllVolumes(0.8f, 0.8f, 0.8f, 0.8f, 0.8f); }

// ========== 查询 ==========
bool UAudioManager::IsSoundPlaying(FName ID) const
{
    for (auto& Pair : ActiveComponents) if (Pair.Value == ID && IsValid(Pair.Key) && Pair.Key->IsPlaying()) return true; return false;
}
int32 UAudioManager::GetActiveSoundCount() const { return ActiveComponents.Num(); }
int32 UAudioManager::GetActiveSoundCountByCategory(EAudioCategory Cat) const
{
    int32 Cnt = 0; for (auto& Pair : ActiveComponents) if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value)) if (Cfg->Category == Cat) ++Cnt; return Cnt;
}
bool UAudioManager::IsVoicePlaying() const { return IsValid(CurrentVoiceComponent) && CurrentVoiceComponent->IsPlaying(); }

// ========== 调试 ==========
void UAudioManager::PrintAudioSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== Audio System Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Active Sounds: %d"), ActiveComponents.Num());
    TArray<EAudioCategory> Cats = { EAudioCategory::BGM, EAudioCategory::SFX, EAudioCategory::Ambient, EAudioCategory::Voice, EAudioCategory::UI };
    TMap<EAudioCategory, int32> Counts;
    for (EAudioCategory C : Cats) Counts.Add(C, 0);
    for (auto& Pair : ActiveComponents) if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value)) Counts[Cfg->Category]++;
    for (EAudioCategory C : Cats) UE_LOG(LogTemp, Log, TEXT("  %s: %d"), *UEnum::GetValueAsString(C), Counts[C]);
    UE_LOG(LogTemp, Log, TEXT("Current BGM: %s"), IsValid(CurrentBGMComponent) ? TEXT("Playing") : TEXT("None"));
    UE_LOG(LogTemp, Log, TEXT("Current Voice: %s"), IsValid(CurrentVoiceComponent) ? TEXT("Playing") : TEXT("None"));
}
void UAudioManager::PrintCategoryStatus(EAudioCategory Cat)
{
    UE_LOG(LogTemp, Log, TEXT("=== %s Audio Status ==="), *UEnum::GetValueAsString(Cat));
    int32 Cnt = 0;
    for (auto& Pair : ActiveComponents)
        if (const FAudioConfig* Cfg = GetAudioConfig(Pair.Value))
            if (Cfg->Category == Cat)
            {
                UE_LOG(LogTemp, Log, TEXT("  - %s"), *Pair.Value.ToString()); ++Cnt;
            }
    UE_LOG(LogTemp, Log, TEXT("Total: %d sounds"), Cnt);
}

// ========== Shutdown（最终安全版本） ==========
void UAudioManager::Shutdown()
{
    CurrentBGMComponent = nullptr;
    CurrentVoiceComponent = nullptr;
    ActiveComponents.Empty();
    LoadedSoundCache.Empty();
    ShutdownEffectSystem();
    UE_LOG(LogTemp, Log, TEXT("AudioManager Shutdown completed"));
}

// ========== 内部辅助 ==========
UWorld* UAudioManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
                return Context.World();
    }
    return nullptr;
}

void UAudioManager::FadeOutAndDestroyAudioComponent(UAudioComponent* AudioComponent, float FadeTime)
{
    if (!IsValid(AudioComponent)) return;

    AudioComponent->FadeOut(FadeTime, 0.0f);

    TWeakObjectPtr<UAudioComponent> WeakComp(AudioComponent);
    TWeakObjectPtr<UAudioManager> WeakThis(this);
    FTimerHandle TimerHandle;

    if (UWorld* World = AudioComponent->GetWorld())
    {
        World->GetTimerManager().SetTimer(TimerHandle, [WeakThis, WeakComp]()
            {
                if (!WeakThis.IsValid()) return;
                UAudioManager* Manager = WeakThis.Get();
                UAudioComponent* Comp = WeakComp.Get();
                if (!IsValid(Comp)) return;

                Manager->ActiveComponents.Remove(Comp);
                if (Comp == Manager->CurrentVoiceComponent) Manager->CurrentVoiceComponent = nullptr;
                if (Comp == Manager->CurrentBGMComponent) Manager->CurrentBGMComponent = nullptr;
                Comp->DestroyComponent();
            }, FadeTime, false);
        PendingDestroyTimers.Add(TimerHandle);
    }
    else
    {
        SafelyDestroyAudioComponent(AudioComponent);
    }
}

void UAudioManager::SafelyDestroyAudioComponent(UAudioComponent* AudioComponent)
{
    if (!IsValid(AudioComponent)) return;
    ActiveComponents.Remove(AudioComponent);
    if (AudioComponent == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
    if (AudioComponent == CurrentBGMComponent) CurrentBGMComponent = nullptr;
    AudioComponent->DestroyComponent();
}

// ========== 音频效果系统 ==========
void UAudioManager::InitializeEffectSystem(UDataTable* InEffectPresetTable)
{
    EffectPresetTable = InEffectPresetTable;
    if (UWorld* World = GetWorld())
        World->GetTimerManager().SetTimer(EffectTickTimerHandle, this, &UAudioManager::TickEffectControllers, 0.016f, true);
    UE_LOG(LogTemp, Log, TEXT("AudioManager effect system initialized"));
}
UAudioEffectController* UAudioManager::GetOrCreateEffectController(UAudioComponent* AudioComponent)
{
    if (!AudioComponent) return nullptr;
    if (TObjectPtr<UAudioEffectController>* Found = ComponentEffectControllers.Find(AudioComponent)) return *Found;
    UAudioEffectController* NewController = NewObject<UAudioEffectController>(this);
    NewController->Initialize(AudioComponent);
    ComponentEffectControllers.Add(AudioComponent, NewController);
    return NewController;
}
void UAudioManager::ApplyEffectPresetBySoundID(FName SoundID, FName EffectPresetRowName)
{
    if (!EffectPresetTable) { UE_LOG(LogTemp, Warning, TEXT("EffectPresetTable not set")); return; }
    const FAudioEffectPreset* Preset = EffectPresetTable->FindRow<FAudioEffectPreset>(EffectPresetRowName, TEXT(""));
    if (!Preset) { UE_LOG(LogTemp, Warning, TEXT("Preset '%s' not found"), *EffectPresetRowName.ToString()); return; }
    TArray<UAudioComponent*> Targets;
    for (auto& Pair : ActiveComponents) if (Pair.Value == SoundID) Targets.Add(Pair.Key);
    for (UAudioComponent* C : Targets) GetOrCreateEffectController(C)->ApplyEffectPreset(*Preset);
}
void UAudioManager::SetSoundLowPassCutoff(FName SoundID, float Cutoff, float Resonance)
{
    for (auto& Pair : ActiveComponents) if (Pair.Value == SoundID) GetOrCreateEffectController(Pair.Key)->SetLowPassCutoff(Cutoff, Resonance);
}
void UAudioManager::StartSoundLowPassLerp(FName SoundID, float TargetCutoff, float Duration)
{
    for (auto& Pair : ActiveComponents) if (Pair.Value == SoundID) GetOrCreateEffectController(Pair.Key)->StartLowPassCutoffLerp(TargetCutoff, Duration);
}
void UAudioManager::ClearSoundEffect(FName SoundID)
{
    for (auto& Pair : ActiveComponents)
        if (Pair.Value == SoundID)
            if (TObjectPtr<UAudioEffectController>* Ctrl = ComponentEffectControllers.Find(Pair.Key))
            {
                (*Ctrl)->ClearEffect(); ComponentEffectControllers.Remove(Pair.Key);
            }
}
void UAudioManager::TickEffectControllers()
{
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    for (auto& Pair : ComponentEffectControllers) if (Pair.Value && IsValid(Pair.Key)) Pair.Value->Tick(DeltaTime);
    TArray<UAudioComponent*> ToRemove;
    for (auto& Pair : ComponentEffectControllers) if (!IsValid(Pair.Key)) ToRemove.Add(Pair.Key);
    for (UAudioComponent* C : ToRemove) ComponentEffectControllers.Remove(C);
}
void UAudioManager::ShutdownEffectSystem()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(EffectTickTimerHandle);
    for (auto& Pair : ComponentEffectControllers) if (Pair.Value) Pair.Value->ClearEffect();
    ComponentEffectControllers.Empty();
    EffectPresetTable = nullptr;
    UE_LOG(LogTemp, Log, TEXT("AudioManager effect system shutdown"));
}
