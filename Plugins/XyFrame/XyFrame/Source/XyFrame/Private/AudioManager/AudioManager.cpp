// Fill out your copyright notice in the Page Settings.
#include "AudioManager/AudioManager.h"
#include "AudioManager/AudioEffectController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "UObject/SoftObjectPath.h"

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
    if (Loaded)
    {
        LoadedSoundCache.Add(AssetPath, Loaded);
    }
    return Loaded;
}

USoundBase* UAudioManager::GetRandomSoundFromConfig(const FAudioConfig* Config)
{
    if (!Config)
        return nullptr;

    // 优先使用随机池
    if (Config->SoundAssets.Num() > 0)
    {
        // 过滤掉无效的空软引用
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

    // 回退到单个资源
    if (!Config->SoundAsset.IsNull())
    {
        return LoadSoundAsset(Config->SoundAsset);
    }

    return nullptr;
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
            TWeakObjectPtr<UAudioManager> WeakThis(this);
            TimerDel.BindLambda([WeakThis, WorldContextObject, SoundID, AttachActor, Location, FadeInTime, PitchMultiplier]()
                {
                    if (WeakThis.IsValid())
                    {
                        WeakThis->PlaySound(WorldContextObject, SoundID, AttachActor, Location, FadeInTime, 0.0f, PitchMultiplier);
                    }
                });
            World->GetTimerManager().SetTimer(TimerHandle, TimerDel, Delay, false);
            PendingDestroyTimers.Add(TimerHandle);
        }
        return nullptr;
    }

    const FAudioConfig* Config = GetAudioConfig(SoundID);
    if (!Config)
    {
        UE_LOG(LogTemp, Error, TEXT("SoundID %s not found in data table!"), *SoundID.ToString());
        return nullptr;
    }

    USoundBase* SoundAsset = GetRandomSoundFromConfig(Config);
    if (!SoundAsset)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid sound asset for SoundID %s (random pool empty and single asset missing)"), *SoundID.ToString());
        return nullptr;
    }

    // 自动停止同类声音（Voice 和 BGM）
    if (Config->Category == EAudioCategory::Voice || Config->Category == EAudioCategory::BGM)
    {
        TArray<UAudioComponent*> ComponentsToStop;
        for (auto& Pair : ActiveComponents)
        {
            const FAudioConfig* OtherConfig = GetAudioConfig(Pair.Value);
            if (OtherConfig && OtherConfig->Category == Config->Category)
            {
                ComponentsToStop.Add(Pair.Key);
            }
        }
        for (UAudioComponent* Comp : ComponentsToStop)
        {
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component == CurrentVoiceComponent) CurrentVoiceComponent = nullptr;
            if (Component == CurrentBGMComponent) CurrentBGMComponent = nullptr;
            if (IsValid(Component)) Component->DestroyComponent();
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
            if (IsValid(Component))
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (Component == CurrentVoiceComponent && Category == EAudioCategory::Voice) CurrentVoiceComponent = nullptr;
            if (Component == CurrentBGMComponent && Category == EAudioCategory::BGM) CurrentBGMComponent = nullptr;
            if (IsValid(Component)) Component->DestroyComponent();
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (IsValid(Component)) Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllSFX(float FadeOutTime) { StopAllSoundsByCategory(EAudioCategory::SFX, FadeOutTime); }

// ========== BGM ==========
void UAudioManager::PlayBGM(UObject* WorldContextObject, FName SoundID, float FadeTime)
{
    float ActualFadeIn = FadeTime;
    if (ActualFadeIn < 0.0f)
    {
        const FAudioConfig* Config = GetAudioConfig(SoundID);
        if (Config && Config->FadeInTime > 0.0f)
            ActualFadeIn = Config->FadeInTime;
        else
            ActualFadeIn = 0.0f;
    }
    PlaySound(WorldContextObject, SoundID, nullptr, FVector::ZeroVector, ActualFadeIn);
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
            if (IsValid(CurrentBGMComponent)) CurrentBGMComponent->Stop();
            ActiveComponents.Remove(CurrentBGMComponent);
            if (IsValid(CurrentBGMComponent)) CurrentBGMComponent->DestroyComponent();
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (IsValid(Component)) Component->DestroyComponent();
        }
    }
}

void UAudioManager::StopAllAmbient(float FadeTime) { StopAllSoundsByCategory(EAudioCategory::Ambient, FadeTime); }

// ========== Voice ==========
void UAudioManager::PlayVoice(
    UObject* WorldContextObject,
    FName SoundID,
    AActor* AttachActor,
    float FadeInTime,
    float FadeOutTime,
    float PitchMultiplier)
{
    (void)FadeOutTime;
    float ActualFadeIn = FadeInTime;
    if (ActualFadeIn < 0.0f)
    {
        const FAudioConfig* NewConfig = GetAudioConfig(SoundID);
        if (NewConfig && NewConfig->FadeInTime > 0.0f)
            ActualFadeIn = NewConfig->FadeInTime;
        else
            ActualFadeIn = 0.0f;
    }
    PlaySound(WorldContextObject, SoundID, AttachActor, FVector::ZeroVector, ActualFadeIn, 0.0f, PitchMultiplier);
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (IsValid(Component)) Component->DestroyComponent();
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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (IsValid(Component)) Component->DestroyComponent();
        }
    }
    CurrentVoiceComponent = nullptr;
}

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
            if (IsValid(Component)) Component->Stop();
            ActiveComponents.Remove(Component);
            if (IsValid(Component)) Component->DestroyComponent();
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

// ========== Shutdown 安全清理 ==========
void UAudioManager::Shutdown()
{
    for (FTimerHandle& Handle : PendingDestroyTimers)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(Handle);
        }
    }
    PendingDestroyTimers.Empty();

    TArray<UAudioComponent*> ComponentsToDestroy;
    ActiveComponents.GenerateKeyArray(ComponentsToDestroy);
    for (UAudioComponent* Comp : ComponentsToDestroy)
    {
        if (IsValid(Comp))
        {
            Comp->Stop();
            Comp->DestroyComponent();
        }
    }
    ActiveComponents.Empty();
    CurrentBGMComponent = nullptr;
    CurrentVoiceComponent = nullptr;

    ShutdownEffectSystem();

    // 清理资源缓存
    LoadedSoundCache.Empty();

    UE_LOG(LogTemp, Log, TEXT("AudioManager Shutdown completed"));
}

// ========== Internal ==========
UWorld* UAudioManager::GetWorld() const
{
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
                return Context.World();
        }
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
    {
        World->GetTimerManager().SetTimer(EffectTickTimerHandle, this, &UAudioManager::TickEffectControllers, 0.016f, true);
    }

    UE_LOG(LogTemp, Log, TEXT("AudioManager effect system initialized"));
}

UAudioEffectController* UAudioManager::GetOrCreateEffectController(UAudioComponent* AudioComponent)
{
    if (!AudioComponent) return nullptr;

    if (TObjectPtr<UAudioEffectController>* Found = ComponentEffectControllers.Find(AudioComponent))
    {
        return *Found;
    }

    UAudioEffectController* NewController = NewObject<UAudioEffectController>(this);
    NewController->Initialize(AudioComponent);
    ComponentEffectControllers.Add(AudioComponent, NewController);
    return NewController;
}

void UAudioManager::ApplyEffectPresetBySoundID(FName SoundID, FName EffectPresetRowName)
{
    if (!EffectPresetTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: EffectPresetTable not set"));
        return;
    }

    const FAudioEffectPreset* Preset = EffectPresetTable->FindRow<FAudioEffectPreset>(EffectPresetRowName, TEXT(""));
    if (!Preset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioManager: Effect preset row '%s' not found"), *EffectPresetRowName.ToString());
        return;
    }

    TArray<UAudioComponent*> TargetComponents;
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            TargetComponents.Add(Pair.Key);
        }
    }

    for (UAudioComponent* Comp : TargetComponents)
    {
        UAudioEffectController* Controller = GetOrCreateEffectController(Comp);
        Controller->ApplyEffectPreset(*Preset);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied effect preset '%s' to %d instances of sound '%s'"),
        *EffectPresetRowName.ToString(), TargetComponents.Num(), *SoundID.ToString());
}

void UAudioManager::SetSoundLowPassCutoff(FName SoundID, float Cutoff, float Resonance)
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            UAudioEffectController* Controller = GetOrCreateEffectController(Pair.Key);
            Controller->SetLowPassCutoff(Cutoff, Resonance);
        }
    }
}

void UAudioManager::StartSoundLowPassLerp(FName SoundID, float TargetCutoff, float Duration)
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            UAudioEffectController* Controller = GetOrCreateEffectController(Pair.Key);
            Controller->StartLowPassCutoffLerp(TargetCutoff, Duration);
        }
    }
}

void UAudioManager::ClearSoundEffect(FName SoundID)
{
    for (auto& Pair : ActiveComponents)
    {
        if (Pair.Value == SoundID)
        {
            if (TObjectPtr<UAudioEffectController>* Ctrl = ComponentEffectControllers.Find(Pair.Key))
            {
                (*Ctrl)->ClearEffect();
                ComponentEffectControllers.Remove(Pair.Key);
            }
        }
    }
}

void UAudioManager::TickEffectControllers()
{
    UWorld* World = GetWorld();
    float DeltaTime = World ? World->GetDeltaSeconds() : 0.016f;

    for (auto& Pair : ComponentEffectControllers)
    {
        if (Pair.Value && IsValid(Pair.Key))
        {
            Pair.Value->Tick(DeltaTime);
        }
    }

    TArray<UAudioComponent*> ToRemove;
    for (auto& Pair : ComponentEffectControllers)
    {
        if (!IsValid(Pair.Key))
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (UAudioComponent* Comp : ToRemove)
    {
        ComponentEffectControllers.Remove(Comp);
    }
}

void UAudioManager::ShutdownEffectSystem()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EffectTickTimerHandle);
    }

    for (auto& Pair : ComponentEffectControllers)
    {
        if (Pair.Value)
        {
            Pair.Value->ClearEffect();
        }
    }
    ComponentEffectControllers.Empty();
    EffectPresetTable = nullptr;

    UE_LOG(LogTemp, Log, TEXT("AudioManager effect system shutdown"));
}