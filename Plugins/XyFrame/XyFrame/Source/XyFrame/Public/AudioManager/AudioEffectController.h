// AudioEffectController.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AudioManager.h"  // 包含 FAudioEffectPreset
#include "AudioEffectController.generated.h"

class UAudioComponent;

// 委托声明 —— 全局作用域
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEffectLerpFinished, bool, IsFinished);

// 普通结构体（用于内部 Lerp 任务）
struct FAudioEffectLerpTask
{
    float StartValue = 0.f;
    float TargetValue = 0.f;
    float Duration = 0.f;
    float ElapsedTime = 0.f;
    bool bRunning = false;
    FOnEffectLerpFinished OnFinished;
};

UCLASS(BlueprintType, Blueprintable)
class XYFRAME_API UAudioEffectController : public UObject
{
    GENERATED_BODY()

public:
    UAudioEffectController();

    UFUNCTION(BlueprintCallable, Category = "AudioEffect")
    void Initialize(UAudioComponent* InTargetComponent);

    UFUNCTION(BlueprintCallable, Category = "AudioEffect")
    void ApplyEffectPreset(const FAudioEffectPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "AudioEffect|Filter")
    void SetLowPassCutoff(float InCutoffFrequency, float InResonance = 1.0f);

    // 蓝图调用版本（必须提供委托）
    UFUNCTION(BlueprintCallable, Category = "AudioEffect|Filter")
    void StartLowPassCutoffLerp(float TargetFrequency, float Duration, FOnEffectLerpFinished OnFinished);

    // C++ 便捷重载（自动传空委托）
    void StartLowPassCutoffLerp(float TargetFrequency, float Duration)
    {
        StartLowPassCutoffLerp(TargetFrequency, Duration, FOnEffectLerpFinished());
    }

    UFUNCTION(BlueprintCallable, Category = "AudioEffect")
    void StopLerp();

    UFUNCTION(BlueprintCallable, Category = "AudioEffect")
    void ClearEffect();

    void Tick(float DeltaTime);

    UAudioComponent* GetTargetComponent() const { return TargetComponent.Get(); }
    bool IsInitialized() const { return bInitialized; }

protected:
    void ApplyFilterParameters(float Cutoff, float Resonance);
    void UpdateLerp(float DeltaTime);

private:
    TWeakObjectPtr<UAudioComponent> TargetComponent;

    float CurrentCutoff;
    float CurrentResonance;

    FAudioEffectLerpTask CutoffLerpTask;

    bool bInitialized;
    bool bFilterActive;   // 记录滤波器是否已启用
};