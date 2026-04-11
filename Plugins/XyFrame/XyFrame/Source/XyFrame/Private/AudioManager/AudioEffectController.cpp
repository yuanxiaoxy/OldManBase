// AudioEffectController.cpp
#include "AudioManager/AudioEffectController.h"
#include "Components/AudioComponent.h"

UAudioEffectController::UAudioEffectController()
    : CurrentCutoff(5000.f)
    , CurrentResonance(1.f)
    , bInitialized(false)
    , bFilterActive(false)
{
}

void UAudioEffectController::Initialize(UAudioComponent* InTargetComponent)
{
    if (!InTargetComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectController: Invalid target component"));
        return;
    }
    TargetComponent = InTargetComponent;
    bInitialized = true;
}

void UAudioEffectController::ApplyEffectPreset(const FAudioEffectPreset& Preset)
{
    if (!bInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("AudioEffectController: Not initialized"));
        return;
    }

    switch (Preset.EffectType)
    {
    case EAudioEffectType::LowPassFilter:
        SetLowPassCutoff(Preset.InitialCutoffFrequency, Preset.InitialResonance);
        break;
    default:
        break;
    }
}

void UAudioEffectController::SetLowPassCutoff(float InCutoffFrequency, float InResonance)
{
    if (!bInitialized) return;

    CurrentCutoff = InCutoffFrequency;
    CurrentResonance = InResonance;

    UAudioComponent* Comp = TargetComponent.Get();
    if (!Comp) return;

    // 启用低通滤波器
    if (!bFilterActive)
    {
        Comp->SetLowPassFilterEnabled(true);
        bFilterActive = true;
    }

    // 设置截止频率（共振在 AudioComponent 原生接口中不受支持，此处忽略 Resonance）
    // 如果需要 Resonance，需要扩展，但通常 AudioComponent 的低通只支持频率
    Comp->SetLowPassFilterFrequency(CurrentCutoff);
}

void UAudioEffectController::ApplyFilterParameters(float Cutoff, float Resonance)
{
    // 直接调用 SetLowPassCutoff 即可
    SetLowPassCutoff(Cutoff, Resonance);
}

void UAudioEffectController::StartLowPassCutoffLerp(float TargetFrequency, float Duration, FOnEffectLerpFinished OnFinished)
{
    if (!bInitialized) return;

    UAudioComponent* Comp = TargetComponent.Get();
    if (!Comp) return;

    // 确保滤波器已启用
    if (!bFilterActive)
    {
        Comp->SetLowPassFilterEnabled(true);
        bFilterActive = true;
    }

    // 停止正在运行的 Lerp
    if (CutoffLerpTask.bRunning && CutoffLerpTask.OnFinished.IsBound())
    {
        CutoffLerpTask.OnFinished.Execute(true);
    }

    CutoffLerpTask.StartValue = CurrentCutoff;
    CutoffLerpTask.TargetValue = TargetFrequency;
    CutoffLerpTask.Duration = FMath::Max(0.001f, Duration);
    CutoffLerpTask.ElapsedTime = 0.f;
    CutoffLerpTask.bRunning = true;
    CutoffLerpTask.OnFinished = OnFinished;
}

void UAudioEffectController::StopLerp()
{
    if (CutoffLerpTask.bRunning)
    {
        CutoffLerpTask.bRunning = false;
        if (CutoffLerpTask.OnFinished.IsBound())
        {
            CutoffLerpTask.OnFinished.Execute(true);
        }
    }
}

void UAudioEffectController::ClearEffect()
{
    StopLerp();

    UAudioComponent* Comp = TargetComponent.Get();
    if (Comp && bFilterActive)
    {
        Comp->SetLowPassFilterEnabled(false);
        bFilterActive = false;
    }
    CurrentCutoff = 5000.f;  // 重置为默认值（或根据配置）
}

void UAudioEffectController::Tick(float DeltaTime)
{
    if (!bInitialized) return;
    UpdateLerp(DeltaTime);
}

void UAudioEffectController::UpdateLerp(float DeltaTime)
{
    if (!CutoffLerpTask.bRunning) return;

    CutoffLerpTask.ElapsedTime += DeltaTime;
    float Alpha = FMath::Clamp(CutoffLerpTask.ElapsedTime / CutoffLerpTask.Duration, 0.f, 1.f);
    float NewCutoff = FMath::Lerp(CutoffLerpTask.StartValue, CutoffLerpTask.TargetValue, Alpha);
    SetLowPassCutoff(NewCutoff, CurrentResonance);

    if (Alpha >= 1.f)
    {
        CutoffLerpTask.bRunning = false;
        if (CutoffLerpTask.OnFinished.IsBound())
        {
            CutoffLerpTask.OnFinished.Execute(false);
        }
    }
}