// OldManCameraAnimationTrigger.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "CameraAnimation/OldManCameraAnimationAsset.h"
#include "OldManCameraAnimationTrigger.generated.h"

UCLASS()
class OLDMAN_API AOldManCameraAnimationTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AOldManCameraAnimationTrigger();

protected:
    virtual void BeginPlay() override;

    // 修复函数签名
    UFUNCTION()
    void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION()
    void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    FOldManCameraAnimationData CameraAnimationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    bool bAutoStartOnEnter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    bool bAutoStopOnExit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Animation")
    bool bOneTimeUse = false;

private:
    UPROPERTY()
    bool bHasBeenUsed = false;
};