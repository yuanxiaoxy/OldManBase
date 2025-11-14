// OldManCameraAnimationTrigger.cpp
#include "CameraAnimation/OldManCameraAnimationTrigger.h"
#include "Character/OldManCharacter.h"
#include "CameraAnimation/OldManCameraAnimationComponent.h"
#include "Components/BoxComponent.h"

AOldManCameraAnimationTrigger::AOldManCameraAnimationTrigger()
{
    // 启用碰撞事件
    GetCollisionComponent()->SetGenerateOverlapEvents(true);
}

void AOldManCameraAnimationTrigger::BeginPlay()
{
    Super::BeginPlay();

    // 绑定重叠事件 - 修复函数签名
    OnActorBeginOverlap.AddDynamic(this, &AOldManCameraAnimationTrigger::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AOldManCameraAnimationTrigger::OnOverlapEnd);
}

// 修复函数签名 - 使用AActor版本而不是UPrimitiveComponent版本
void AOldManCameraAnimationTrigger::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    AOldManCharacter* Character = Cast<AOldManCharacter>(OtherActor);
    if (!Character || (bOneTimeUse && bHasBeenUsed)) return;

    UOldManCameraAnimationComponent* AnimationComponent = Character->FindComponentByClass<UOldManCameraAnimationComponent>();
    if (AnimationComponent && bAutoStartOnEnter)
    {
        AnimationComponent->StartCameraAnimation(CameraAnimationData);
        bHasBeenUsed = true;

        UE_LOG(LogTemp, Log, TEXT("Camera animation triggered by: %s"), *Character->GetName());
    }
}

// 修复函数签名 - 使用AActor版本而不是UPrimitiveComponent版本
void AOldManCameraAnimationTrigger::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
    AOldManCharacter* Character = Cast<AOldManCharacter>(OtherActor);
    if (!Character) return;

    UOldManCameraAnimationComponent* AnimationComponent = Character->FindComponentByClass<UOldManCameraAnimationComponent>();
    if (AnimationComponent && bAutoStopOnExit)
    {
        AnimationComponent->StopCameraAnimation();

        UE_LOG(LogTemp, Log, TEXT("Camera animation stopped for: %s"), *Character->GetName());
    }
}