#include "CameraAnimation/OldManCameraAnimationComponent.h"
#include "Character/OldManCameraComponent.h"
#include "Character/OldManCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Camera/CameraActor.h"
#include "Character/OldManPersonPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CameraAnimation/OldManCameraAnimationTrigger.h"

UOldManCameraAnimationComponent::UOldManCameraAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOldManCameraAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOldManCameraAnimationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsAnimationPlaying && !bIsPaused)
	{
		UpdateCameraAnimation(DeltaTime);
	}
}

void UOldManCameraAnimationComponent::InitializeCameraAnimation(UOldManCameraComponent* InCameraComponent, AOldManCharacter* InCharacter)
{
	CameraComponent = InCameraComponent;
	Character = InCharacter;
}

void UOldManCameraAnimationComponent::StartCameraAnimation(const FOldManCameraAnimationData& AnimationData, bool bForceRestart)
{
	if (!CameraComponent || !Character)
	{
		UE_LOG(LogTemp, Error, TEXT("CameraAnimationComponent not properly initialized"));
		return;
	}

	// 如果已经有动画在播放
	if (bIsAnimationPlaying && !bForceRestart)
	{
		// 使用切换动画功能，而不是直接开始
		SwitchCameraAnimation(AnimationData, AnimationData.BlendInTime);
		return;
	}

	// 如果是强制重启或没有动画在播放，直接开始
	CurrentAnimationData = AnimationData;
	bIsAnimationPlaying = true;
	bIsPaused = false;
	bIsBlendingIn = true;
	bIsBlendingOut = false;
	bIsSwitchingAnimation = false;
	bFirstFrameAfterSwitch = false;
	AnimationBlendAlpha = 0.0f;
	AnimationInternalBlendAlpha = 0.0f;
	AnimationElapsedTime = 0.0f;
	ExitBlendTime = 0.0f;
	ExitBlendElapsed = 0.0f;
	SwitchTransitionTime = 0.0f;
	SwitchTransitionElapsed = 0.0f;

	// 保存原始状态
	OriginalFOV = CameraComponent->CurCameraFOV;

	// 保存相机相对于玩家的相对位置和旋转
	if (Character && CameraComponent)
	{
		FVector CameraWorldLocation = CameraComponent->GetCameraLocation();
		FVector CharacterWorldLocation = Character->GetActorLocation();

		// 计算相机相对于玩家的偏移
		OriginalCameraRelativeOffset = Character->GetActorTransform().InverseTransformPosition(CameraWorldLocation);

		// 计算相机相对于玩家的旋转
		FRotator CameraWorldRotation = CameraComponent->GetCameraRotation();
		FRotator CharacterWorldRotation = Character->GetActorRotation();

		// 使用四元数来计算相对旋转
		FQuat CharacterQuat = CharacterWorldRotation.Quaternion();
		FQuat CameraQuat = CameraWorldRotation.Quaternion();
		FQuat RelativeQuat = CharacterQuat.Inverse() * CameraQuat;
		OriginalCameraRelativeRotation = RelativeQuat.Rotator();

		// 初始化平滑插值位置和旋转
		FVector InitialCameraLocation = CalculateAnimatedCameraLocation();
		FRotator InitialCameraRotation = CalculateAnimatedCameraRotation();

		LerpCameraLocation = InitialCameraLocation;
		LerpCameraRotation = InitialCameraRotation;
	}

	// 初始化轨道参数
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::OrbitAroundObject)
	{
		CurrentOrbitRadius = CurrentAnimationData.OrbitRadius;
		CurrentOrbitHeight = CurrentAnimationData.OrbitHeight;
		CurrentOrbitSpeed = CurrentAnimationData.OrbitSpeed;
		bCurrentClockwiseOrbit = CurrentAnimationData.bClockwiseOrbit;
		CurrentOrbitAngle = 0.0f;
	}

	// 创建动画相机
	CreateCameraAnimationActor();
	SwitchToAnimationCamera();

	// 应用FOV覆盖
	if (CurrentAnimationData.bOverrideFOV)
	{
		ApplyFOV(CurrentAnimationData.FOV);
	}

	// 处理玩家输入和可见性
	if (CurrentAnimationData.bDisablePlayerInput)
	{
		DisablePlayerInput();
	}

	// 处理鼠标位置暴露
	HandleMouseExposure(CurrentAnimationData.bExposeMousePosition);

	if (CurrentAnimationData.bHidePlayer)
	{
		bOriginalPlayerVisibility = !Character->IsHidden();
		Character->SetActorHiddenInGame(true);

		// 隐藏网格组件
		if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
		{
			MeshComponent->SetVisibility(false);
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		Character->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
		{
			MeshComponent->SetVisibility(false);
		}
	}

	OnCameraAnimationStarted.Broadcast(CurrentAnimationData);

	// 启动序列（如果是序列类型）
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::LevelSequence)
	{
		StartLevelSequence();
	}
}

void UOldManCameraAnimationComponent::SwitchCameraAnimation(const FOldManCameraAnimationData& NewAnimationData, float TransitionTime)
{
	if (!bIsAnimationPlaying)
	{
		// 如果没有动画在播放，直接开始
		StartCameraAnimation(NewAnimationData, true);
		return;
	}

	// 保存当前动画状态
	SaveCurrentAnimationState();

	// 设置切换参数
	bIsSwitchingAnimation = true;
	bFirstFrameAfterSwitch = false;
	SwitchTransitionTime = TransitionTime;
	SwitchTransitionElapsed = 0.0f;
	SwitchTargetData = NewAnimationData;

	// 保存切换起始位置
	if (CameraAnimationActor)
	{
		SwitchStartLocation = CameraAnimationActor->GetActorLocation();
		SwitchStartRotation = CameraAnimationActor->GetActorRotation();
	}
	else
	{
		SwitchStartLocation = CalculateAnimatedCameraLocation();
		SwitchStartRotation = CalculateAnimatedCameraRotation();
	}

	// 保存当前动画数据作为上一个动画
	PreviousAnimationData = CurrentAnimationData;

	// 更新当前动画数据
	CurrentAnimationData = NewAnimationData;

	// 重置混合状态
	bIsBlendingIn = true;
	AnimationInternalBlendAlpha = 0.0f;
	AnimationBlendAlpha = 0.0f;

	// 初始化新动画的参数
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::OrbitAroundObject)
	{
		CurrentOrbitRadius = CurrentAnimationData.OrbitRadius;
		CurrentOrbitHeight = CurrentAnimationData.OrbitHeight;
		CurrentOrbitSpeed = CurrentAnimationData.OrbitSpeed;
		bCurrentClockwiseOrbit = CurrentAnimationData.bClockwiseOrbit;
		CurrentOrbitAngle = 0.0f;
	}

	// 重置动画时间
	AnimationElapsedTime = 0.0f;

	// 初始化Lerp值
	LerpCameraLocation = SwitchStartLocation;
	LerpCameraRotation = SwitchStartRotation;

	// 处理鼠标位置暴露
	HandleMouseExposure(CurrentAnimationData.bExposeMousePosition);

	// 广播动画切换事件
	OnCameraAnimationStarted.Broadcast(CurrentAnimationData);
}

void UOldManCameraAnimationComponent::SaveCurrentAnimationState()
{
	if (CameraAnimationActor)
	{
		PreviousAnimationEndLocation = CameraAnimationActor->GetActorLocation();
		PreviousAnimationEndRotation = CameraAnimationActor->GetActorRotation();
	}
	else
	{
		PreviousAnimationEndLocation = CalculateAnimatedCameraLocation();
		PreviousAnimationEndRotation = CalculateAnimatedCameraRotation();
	}
}

void UOldManCameraAnimationComponent::SetAnimationTarget(AActor* TargetActor)
{
	if (!bIsAnimationPlaying) return;

	CurrentAnimationData.SetRuntimeTarget(TargetActor);
}

void UOldManCameraAnimationComponent::StopCameraAnimation(bool bImmediate)
{
	if (!bIsAnimationPlaying) return;

	if (bImmediate)
	{
		bIsAnimationPlaying = false;
		bIsBlendingOut = false;
		bIsSwitchingAnimation = false;
		bFirstFrameAfterSwitch = false;

		// 恢复鼠标位置状态
		if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayer && CurrentAnimationData.bExposeMousePosition)
		{
			HandleMouseExposure(false);
		}

		StopLevelSequence();
		SwitchBackToPlayerCamera();
		DestroyCameraAnimationActor();
		EnablePlayerInput();
		ApplyFOV(OriginalFOV);
		CurrentStopTrigger = nullptr;

		OnCameraAnimationFinished.Broadcast();
		return;
	}

	// 开始退出动画的平滑过渡
	StartExitBlend();
}

void UOldManCameraAnimationComponent::PauseCameraAnimation()
{
	if (!bIsAnimationPlaying) return;

	bIsPaused = true;

	if (SequencePlayer && SequencePlayer->IsPlaying())
	{
		SequencePlayer->Pause();
	}

	OnCameraAnimationPaused.Broadcast();
}

void UOldManCameraAnimationComponent::SetRuntimeFollowTarget(AActor* FollowTarget)
{
	if (FollowTarget)
	{
		CurrentAnimationData.RuntimeFollowTarget = FollowTarget;
	}
}

bool UOldManCameraAnimationComponent::IsStopTriggerValid(AOldManCameraAnimationTrigger* StopTrigger) const
{
	return CurrentStopTrigger == StopTrigger;
}

void UOldManCameraAnimationComponent::SetOrbitParameters(float Radius, float Height, float Speed, bool bClockwise)
{
	CurrentOrbitRadius = Radius;
	CurrentOrbitHeight = Height;
	CurrentOrbitSpeed = Speed;
	bCurrentClockwiseOrbit = bClockwise;
}

void UOldManCameraAnimationComponent::SetMoveToParameters(const FVector& PositionOffset, ECameraOffsetSpace OffsetSpace)
{
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::MoveToTargetAndLookAtPlayer)
	{
		CurrentAnimationData.MoveToPositionOffset = PositionOffset;
		CurrentAnimationData.MoveToOffsetSpace = OffsetSpace;
	}
}

void UOldManCameraAnimationComponent::SetFollowPlayerParameters(const FVector& CameraOffset, ECameraOffsetSpace OffsetSpace, bool bWithRotation)
{
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayer)
	{
		CurrentAnimationData.FollowCameraOffset = CameraOffset;
		CurrentAnimationData.FollowOffsetSpace = OffsetSpace;
		CurrentAnimationData.bFollowWithRotation = bWithRotation;
	}
}

void UOldManCameraAnimationComponent::SetFollowPlayerAndLookAtParameters(float SphereRadius, float SphereHeight, const FVector& CameraOffset, const FRotator& CameraRotationOffset, bool bUseExtensionLineOffset)
{
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayerAndLookAtTarget)
	{
		CurrentAnimationData.SphereRadius = SphereRadius;
		CurrentAnimationData.SphereHeight = SphereHeight;
		CurrentAnimationData.CameraOffset = CameraOffset;
		CurrentAnimationData.CameraRotationOffset = CameraRotationOffset;
		CurrentAnimationData.bUseExtensionLineOffset = bUseExtensionLineOffset;
	}
}

FVector UOldManCameraAnimationComponent::GetAnimationCameraLocation() const
{
	return CameraAnimationActor ? CameraAnimationActor->GetActorLocation() : FVector::ZeroVector;
}

FRotator UOldManCameraAnimationComponent::GetAnimationCameraRotation() const
{
	return CameraAnimationActor ? CameraAnimationActor->GetActorRotation() : FRotator::ZeroRotator;
}

void UOldManCameraAnimationComponent::SetMovementLerpParameters(bool bUsePositionLerp, float PositionLerpSpeed, bool bUseRotationLerp, float RotationLerpSpeed)
{
	CurrentAnimationData.bUsePositionLerp = bUsePositionLerp;
	CurrentAnimationData.PositionLerpSpeed = PositionLerpSpeed;
	CurrentAnimationData.bUseRotationLerp = bUseRotationLerp;
	CurrentAnimationData.RotationLerpSpeed = RotationLerpSpeed;
}

void UOldManCameraAnimationComponent::SetExposeMousePosition(bool bExposeMouse)
{
	// 只有在FollowPlayer模式下才能设置
	if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayer)
	{
		CurrentAnimationData.bExposeMousePosition = bExposeMouse;
		HandleMouseExposure(bExposeMouse);
	}
}

void UOldManCameraAnimationComponent::UpdateCameraAnimation(float DeltaTime)
{
	// 更新混合
	UpdateBlend(DeltaTime);

	// 如果正在切换动画，更新切换过渡
	if (bIsSwitchingAnimation && SwitchTransitionTime > 0.0f)
	{
		UpdateSwitchTransition(DeltaTime);
		return;
	}

	// 处理切换完成后的第一帧
	if (bFirstFrameAfterSwitch)
	{
		HandleFirstFrameAfterSwitch(DeltaTime);
		return;
	}

	// 如果正在退出动画的混合过程，更新退出混合
	if (ExitBlendTime > 0.0f)
	{
		UpdateExitBlend(DeltaTime);
		return;
	}

	// 更新动画已播放时间
	AnimationElapsedTime += DeltaTime;

	// 根据动画类型更新相机位置和旋转
	switch (CurrentAnimationData.AnimationType)
	{
	case ECameraAnimationType::OrbitAroundObject:
		UpdateOrbitAnimation();
		break;
	case ECameraAnimationType::LookAtObject:
		UpdateLookAtObjectAnimation();
		break;
	case ECameraAnimationType::AlignToTarget:
		UpdateAlignToTargetAnimation();
		break;
	case ECameraAnimationType::MoveToTargetAndLookAtPlayer:
		UpdateMoveToTargetAndLookAtPlayerAnimation();
		break;
	case ECameraAnimationType::FollowPlayer:
		UpdateFollowPlayerAnimation();
		break;
	case ECameraAnimationType::FollowPlayerAndLookAtTarget:
		UpdateFollowPlayerAndLookAtTargetAnimation();
		break;
	}

	// 检查停止条件
	if (CheckStopConditions(DeltaTime))
	{
		return;
	}

	// 更新非序列类型的相机变换
	if (CurrentAnimationData.AnimationType != ECameraAnimationType::LevelSequence && CameraAnimationActor)
	{
		// 计算目标位置和旋转
		FVector TargetLocation = CalculateAnimatedCameraLocation();
		FRotator TargetRotation = CalculateAnimatedCameraRotation();

		// 对于跟随和移动动画，应用位置和旋转平滑插值
		if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayer ||
			CurrentAnimationData.AnimationType == ECameraAnimationType::MoveToTargetAndLookAtPlayer ||
			CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayerAndLookAtTarget)
		{
			if (CurrentAnimationData.bUsePositionLerp)
			{
				TargetLocation = ApplyPositionLerp(DeltaTime, TargetLocation);
			}

			if (CurrentAnimationData.bUseRotationLerp)
			{
				TargetRotation = ApplyRotationLerp(DeltaTime, TargetRotation);
			}
		}

		// 应用曲线控制的混合
		float FinalBlendAlpha = AnimationBlendAlpha;
		if (CurrentAnimationData.BlendCurve)
		{
			FinalBlendAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(AnimationInternalBlendAlpha);
		}

		// 对于进入混合，应用曲线
		if (bIsBlendingIn && CurrentAnimationData.BlendCurve)
		{
			FVector StartLocation = SwitchStartLocation;
			FRotator StartRotation = SwitchStartRotation;

			FVector BlendedLocation = FMath::Lerp(StartLocation, TargetLocation, FinalBlendAlpha);
			FRotator BlendedRotation = FMath::Lerp(StartRotation, TargetRotation, FinalBlendAlpha);

			CameraAnimationActor->SetActorLocation(BlendedLocation);
			CameraAnimationActor->SetActorRotation(BlendedRotation);
		}
		else
		{
			// 直接设置相机变换
			CameraAnimationActor->SetActorLocation(TargetLocation);
			CameraAnimationActor->SetActorRotation(TargetRotation);
		}

		// 更新FOV
		if (CurrentAnimationData.bOverrideFOV && CameraAnimationActor->GetCameraComponent())
		{
			CameraAnimationActor->GetCameraComponent()->FieldOfView = CurrentAnimationData.FOV;
		}
	}
}

void UOldManCameraAnimationComponent::UpdateSwitchTransition(float DeltaTime)
{
	SwitchTransitionElapsed += DeltaTime;
	float SwitchAlpha = FMath::Clamp(SwitchTransitionElapsed / SwitchTransitionTime, 0.0f, 1.0f);

	// 应用曲线控制切换速度
	float CurveAlpha = SwitchAlpha;
	if (CurrentAnimationData.BlendCurve)
	{
		CurveAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(SwitchAlpha);
	}

	// 计算新动画的目标位置和旋转
	FVector NewTargetLocation = CalculateAnimatedCameraLocation();
	FRotator NewTargetRotation = CalculateAnimatedCameraRotation();

	// 保存切换完成后的目标位置
	PostSwitchTargetLocation = NewTargetLocation;
	PostSwitchTargetRotation = NewTargetRotation;

	// 插值位置和旋转
	FVector CurrentLocation = FMath::Lerp(SwitchStartLocation, NewTargetLocation, CurveAlpha);
	FRotator CurrentRotation = FMath::Lerp(SwitchStartRotation, NewTargetRotation, CurveAlpha);

	// 更新动画相机
	if (CameraAnimationActor)
	{
		CameraAnimationActor->SetActorLocation(CurrentLocation);
		CameraAnimationActor->SetActorRotation(CurrentRotation);
	}

	// 插值FOV
	if (CurrentAnimationData.bOverrideFOV && CameraAnimationActor && CameraAnimationActor->GetCameraComponent())
	{
		float PreviousFOV = PreviousAnimationData.bOverrideFOV ? PreviousAnimationData.FOV : OriginalFOV;
		float CurrentFOV = FMath::Lerp(PreviousFOV, CurrentAnimationData.FOV, CurveAlpha);
		CameraAnimationActor->GetCameraComponent()->FieldOfView = CurrentFOV;
	}

	if (SwitchAlpha >= 1.0f)
	{
		// 切换完成，标记为第一帧后切换
		bIsSwitchingAnimation = false;
		bFirstFrameAfterSwitch = true;
		SwitchTransitionTime = 0.0f;
		SwitchTransitionElapsed = 0.0f;

		// 更新内部混合Alpha
		AnimationInternalBlendAlpha = 1.0f;
		AnimationBlendAlpha = 1.0f;
		bIsBlendingIn = false;

		// 确保相机位置准确设置为新动画的目标位置
		if (CameraAnimationActor)
		{
			CameraAnimationActor->SetActorLocation(NewTargetLocation);
			CameraAnimationActor->SetActorRotation(NewTargetRotation);
		}

		// 初始化Lerp值为当前位置，避免下一帧跳跃
		LerpCameraLocation = NewTargetLocation;
		LerpCameraRotation = NewTargetRotation;

		UE_LOG(LogTemp, Log, TEXT("Switch animation completed at location: %s"), *NewTargetLocation.ToString());
	}
}

void UOldManCameraAnimationComponent::HandleFirstFrameAfterSwitch(float DeltaTime)
{
	// 只在第一帧使用切换完成时的位置作为起始点
	if (CameraAnimationActor)
	{
		// 计算当前帧的目标位置
		FVector TargetLocation = CalculateAnimatedCameraLocation();
		FRotator TargetRotation = CalculateAnimatedCameraRotation();

		// 使用LerpCameraLocation和LerpCameraRotation作为起始点（这些值已经在切换完成时设置为正确值）
		if (CurrentAnimationData.bUsePositionLerp)
		{
			TargetLocation = ApplyPositionLerp(DeltaTime, TargetLocation);
		}

		if (CurrentAnimationData.bUseRotationLerp)
		{
			TargetRotation = ApplyRotationLerp(DeltaTime, TargetRotation);
		}

		// 设置相机位置
		CameraAnimationActor->SetActorLocation(TargetLocation);
		CameraAnimationActor->SetActorRotation(TargetRotation);
	}

	// 只执行一帧
	bFirstFrameAfterSwitch = false;
}

void UOldManCameraAnimationComponent::UpdateAlignToTargetAnimation()
{
	// 对齐目标模式：相机位置和旋转完全对齐到目标物体
}

void UOldManCameraAnimationComponent::UpdateLookAtObjectAnimation()
{
	// 看向物体模式：相机位置基于目标物体，旋转看向目标
}

void UOldManCameraAnimationComponent::UpdateOrbitAnimation()
{
	// 轨道模式：更新轨道角度
	UpdateOrbit(GetWorld()->GetDeltaSeconds());
}

void UOldManCameraAnimationComponent::UpdateMoveToTargetAndLookAtPlayerAnimation()
{
	// 移动到目标并看向玩家模式：每帧更新相机位置和旋转
}

void UOldManCameraAnimationComponent::UpdateFollowPlayerAnimation()
{
	// 跟随玩家模式：每帧更新相机位置和旋转
}

void UOldManCameraAnimationComponent::UpdateFollowPlayerAndLookAtTargetAnimation()
{
	// 跟随玩家并看向目标模式：每帧更新相机位置和旋转
}

void UOldManCameraAnimationComponent::UpdateBlend(float DeltaTime)
{
	if (bIsBlendingIn)
	{
		float BlendTime = CurrentAnimationData.BlendInTime > 0.0f ? CurrentAnimationData.BlendInTime : 0.1f;
		AnimationInternalBlendAlpha += DeltaTime / BlendTime;
		AnimationBlendAlpha = AnimationInternalBlendAlpha;

		if (AnimationInternalBlendAlpha >= 1.0f)
		{
			AnimationInternalBlendAlpha = 1.0f;
			AnimationBlendAlpha = 1.0f;
			bIsBlendingIn = false;
		}

		// 应用曲线
		if (CurrentAnimationData.BlendCurve)
		{
			AnimationBlendAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(AnimationInternalBlendAlpha);
		}
	}
	else if (bIsBlendingOut)
	{
		float BlendTime = CurrentAnimationData.BlendOutTime > 0.0f ? CurrentAnimationData.BlendOutTime : 0.1f;
		AnimationInternalBlendAlpha -= DeltaTime / BlendTime;
		AnimationBlendAlpha = AnimationInternalBlendAlpha;

		if (AnimationInternalBlendAlpha <= 0.0f)
		{
			AnimationInternalBlendAlpha = 0.0f;
			AnimationBlendAlpha = 0.0f;
		}

		// 应用曲线
		if (CurrentAnimationData.BlendCurve)
		{
			AnimationBlendAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(AnimationInternalBlendAlpha);
		}
	}
}

void UOldManCameraAnimationComponent::UpdateExitBlend(float DeltaTime)
{
	ExitBlendElapsed += DeltaTime;
	float ExitBlendAlpha = FMath::Clamp(ExitBlendElapsed / ExitBlendTime, 0.0f, 1.0f);

	float CurveAlpha = ExitBlendAlpha;
	if (CurrentAnimationData.BlendCurve)
	{
		CurveAlpha = CurrentAnimationData.BlendCurve->GetFloatValue(ExitBlendAlpha);
	}

	if (CameraAnimationActor && CameraComponent && Character)
	{
		// 目标位置/旋转（回归到原始相机）
		FVector TargetLocation = CalculateTargetCameraLocation();
		FRotator TargetRotation = CalculateTargetCameraRotation();

		// 从记录的固定起点插值到目标
		FVector CurrentLocation = FMath::Lerp(ExitStartLocation, TargetLocation, CurveAlpha);
		FRotator CurrentRotation = FMath::Lerp(ExitStartRotation, TargetRotation, CurveAlpha);

		CameraAnimationActor->SetActorLocation(CurrentLocation);
		CameraAnimationActor->SetActorRotation(CurrentRotation);

		// 插值FOV：从记录的起始FOV到原始FOV
		if (UCameraComponent* CamComp = CameraAnimationActor->GetCameraComponent())
		{
			float TargetFOV = OriginalFOV;
			float CurrentFOV = FMath::Lerp(ExitStartFOV, TargetFOV, CurveAlpha);
			CamComp->FieldOfView = CurrentFOV;
		}
	}

	if (ExitBlendAlpha >= 1.0f)
	{
		// 恢复鼠标位置状态
		if (CurrentAnimationData.AnimationType == ECameraAnimationType::FollowPlayer && CurrentAnimationData.bExposeMousePosition)
		{
			HandleMouseExposure(false);
		}

		// 退出混合完成，执行清理
		ExitBlendTime = 0.0f;
		ExitBlendElapsed = 0.0f;
		bIsAnimationPlaying = false;
		bIsBlendingOut = false;

		StopLevelSequence();
		SwitchBackToPlayerCamera();
		DestroyCameraAnimationActor();
		EnablePlayerInput();
		ApplyFOV(OriginalFOV);
		CurrentStopTrigger = nullptr;

		OnCameraAnimationFinished.Broadcast();
	}
}

void UOldManCameraAnimationComponent::UpdateOrbit(float DeltaTime)
{
	float Direction = bCurrentClockwiseOrbit ? 1.0f : -1.0f;
	CurrentOrbitAngle += CurrentOrbitSpeed * DeltaTime * Direction;

	// 规范化角度
	if (CurrentOrbitAngle >= 360.0f) CurrentOrbitAngle -= 360.0f;
	else if (CurrentOrbitAngle < 0.0f) CurrentOrbitAngle += 360.0f;
}

FVector UOldManCameraAnimationComponent::ApplyPositionLerp(float DeltaTime, const FVector& TargetLocation)
{
	if (!CurrentAnimationData.bUsePositionLerp)
	{
		return TargetLocation;
	}

	// 使用插值平滑过渡到目标位置
	FVector ResultLocation = FMath::VInterpTo(
		LerpCameraLocation,
		TargetLocation,
		DeltaTime,
		CurrentAnimationData.PositionLerpSpeed
	);

	// 保存当前插值位置
	LerpCameraLocation = ResultLocation;

	return ResultLocation;
}

FRotator UOldManCameraAnimationComponent::ApplyRotationLerp(float DeltaTime, const FRotator& TargetRotation)
{
	if (!CurrentAnimationData.bUseRotationLerp)
	{
		return TargetRotation;
	}

	// 使用插值平滑过渡到目标旋转
	FRotator ResultRotation = FMath::RInterpTo(
		LerpCameraRotation,
		TargetRotation,
		DeltaTime,
		CurrentAnimationData.RotationLerpSpeed
	);

	// 保存当前插值旋转
	LerpCameraRotation = ResultRotation;

	return ResultRotation;
}

FVector UOldManCameraAnimationComponent::CalculateOrbitLocation() const
{
	if (!CurrentAnimationData.TargetObject)
	{
		return CalculateTargetCameraLocation();
	}

	FVector TargetLocation = CurrentAnimationData.TargetObject->GetActorLocation();
	float AngleRad = FMath::DegreesToRadians(CurrentOrbitAngle);

	float X = TargetLocation.X + CurrentOrbitRadius * FMath::Cos(AngleRad);
	float Y = TargetLocation.Y + CurrentOrbitRadius * FMath::Sin(AngleRad);
	float Z = TargetLocation.Z + CurrentOrbitHeight;

	return FVector(X, Y, Z);
}

FVector UOldManCameraAnimationComponent::CalculateAnimatedCameraLocation() const
{
	switch (CurrentAnimationData.AnimationType)
	{
	case ECameraAnimationType::LookAtObject:
		if (CurrentAnimationData.TargetObject)
		{
			return CurrentAnimationData.GetLookAtCameraLocation();
		}
		break;

	case ECameraAnimationType::OrbitAroundObject:
		if (CurrentAnimationData.TargetObject)
		{
			return CalculateOrbitLocation();
		}
		break;

	case ECameraAnimationType::AlignToTarget:
		if (CurrentAnimationData.TargetObject)
		{
			return CurrentAnimationData.GetAlignLocation();
		}
		break;

	case ECameraAnimationType::MoveToTargetAndLookAtPlayer:
		if (CurrentAnimationData.TargetObject)
		{
			return CurrentAnimationData.GetMoveToCameraLocation();
		}
		break;

	case ECameraAnimationType::FollowPlayer:
		if (Character)
		{
			return CurrentAnimationData.GetFollowCameraLocation(Character);
		}
		break;

	case ECameraAnimationType::FollowPlayerAndLookAtTarget:
		if (Character)
		{
			return CurrentAnimationData.GetFollowPlayerAndLookAtCameraLocation(Character);
		}
		break;

	case ECameraAnimationType::LevelSequence:
	default:
		break;
	}

	return CalculateTargetCameraLocation();
}

FRotator UOldManCameraAnimationComponent::CalculateAnimatedCameraRotation() const
{
	switch (CurrentAnimationData.AnimationType)
	{
	case ECameraAnimationType::LookAtObject:
	case ECameraAnimationType::OrbitAroundObject:
		if (CurrentAnimationData.TargetObject)
		{
			FVector CameraLocation = CalculateAnimatedCameraLocation();
			FVector TargetLocation = CurrentAnimationData.GetTargetLocation();
			FVector LookDirection = (TargetLocation - CameraLocation).GetSafeNormal();
			if (!LookDirection.IsNearlyZero())
			{
				return LookDirection.Rotation();
			}
		}
		break;

	case ECameraAnimationType::AlignToTarget:
		if (CurrentAnimationData.TargetObject)
		{
			return CurrentAnimationData.GetAlignRotation();
		}
		break;

	case ECameraAnimationType::MoveToTargetAndLookAtPlayer:
		if (CurrentAnimationData.TargetObject && Character)
		{
			return CurrentAnimationData.GetMoveToCameraRotation(Character);
		}
		break;

	case ECameraAnimationType::FollowPlayer:
		if (Character)
		{
			return CurrentAnimationData.GetFollowCameraRotation(Character);
		}
		break;

	case ECameraAnimationType::FollowPlayerAndLookAtTarget:
		if (Character)
		{
			return CurrentAnimationData.GetFollowPlayerAndLookAtCameraRotation(Character);
		}
		break;

	case ECameraAnimationType::LevelSequence:
	default:
		break;
	}

	return CalculateTargetCameraRotation();
}

FVector UOldManCameraAnimationComponent::CalculateTargetCameraLocation() const
{
	// 计算相对于玩家当前位置的目标相机位置
	if (Character)
	{
		return Character->GetActorTransform().TransformPosition(OriginalCameraRelativeOffset);
	}
	return FVector::ZeroVector;
}

FRotator UOldManCameraAnimationComponent::CalculateTargetCameraRotation() const
{
	// 计算相对于玩家当前旋转的目标相机旋转
	if (Character)
	{
		FVector CharacterLocation = Character->GetActorLocation();
		FVector TargetCameraLocation = CalculateTargetCameraLocation();

		// 计算看向人物的旋转
		FVector LookDirection = (CharacterLocation - TargetCameraLocation).GetSafeNormal();
		if (!LookDirection.IsNearlyZero())
		{
			return LookDirection.Rotation();
		}
	}
	return FRotator::ZeroRotator;
}

void UOldManCameraAnimationComponent::CreateCameraAnimationActor()
{
	// 如果已经有动画相机，重新使用它
	if (CameraAnimationActor)
	{
		// 更新位置和旋转
		FVector SpawnLocation = CalculateAnimatedCameraLocation();
		FRotator SpawnRotation = CalculateAnimatedCameraRotation();

		CameraAnimationActor->SetActorLocation(SpawnLocation);
		CameraAnimationActor->SetActorRotation(SpawnRotation);
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = CalculateAnimatedCameraLocation();
	FRotator SpawnRotation = CalculateAnimatedCameraRotation();

	CameraAnimationActor = GetWorld()->SpawnActor<ACameraActor>(
		ACameraActor::StaticClass(),
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (CameraAnimationActor)
	{
		if (CurrentAnimationData.bOverrideFOV)
		{
			CameraAnimationActor->GetCameraComponent()->FieldOfView = CurrentAnimationData.FOV;
		}
		else
		{
			CameraAnimationActor->GetCameraComponent()->FieldOfView = OriginalFOV;
		}

		CameraAnimationActor->Tags.Add(FName("CameraAnimationActor"));
	}
}

void UOldManCameraAnimationComponent::DestroyCameraAnimationActor()
{
	if (CameraAnimationActor)
	{
		CameraAnimationActor->Destroy();
		CameraAnimationActor = nullptr;
	}
}

void UOldManCameraAnimationComponent::SwitchToAnimationCamera()
{
	if (!CameraAnimationActor) return;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		OriginalViewTarget = PlayerController->GetViewTarget();
		PlayerController->SetViewTargetWithBlend(CameraAnimationActor, CurrentAnimationData.BlendInTime);
	}
}

void UOldManCameraAnimationComponent::SwitchBackToPlayerCamera()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController && OriginalViewTarget)
	{
		PlayerController->SetViewTargetWithBlend(OriginalViewTarget, CurrentAnimationData.BlendOutTime);
	}
}

void UOldManCameraAnimationComponent::DisablePlayerInput()
{
	AOldManPersonPlayerController* PlayerController = Character->GetOldManController();
	if (PlayerController)
	{
		bOriginalPlayerInputState = PlayerController->InputEnabled();
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputEnabled(false);
	}
}

void UOldManCameraAnimationComponent::EnablePlayerInput()
{
	AOldManPersonPlayerController* PlayerController = Character->GetOldManController();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = false;
		PlayerController->SetInputEnabled(true);

		if (bOriginalPlayerInputState)
		{
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = false;
		}

		// 恢复玩家可见性
		if (CurrentAnimationData.bHidePlayer && Character)
		{
			Character->SetActorHiddenInGame(!bOriginalPlayerVisibility);

			if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
			{
				MeshComponent->SetVisibility(true);
			}

			TArray<UStaticMeshComponent*> StaticMeshComponents;
			Character->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
			for (UStaticMeshComponent* MeshComponent : StaticMeshComponents)
			{
				MeshComponent->SetVisibility(true);
			}
		}
	}
}

void UOldManCameraAnimationComponent::ApplyFOV(float InFOV)
{
	if (CameraComponent)
	{
		CameraComponent->SetCameraFOV(InFOV);
	}
}

void UOldManCameraAnimationComponent::StartLevelSequence()
{
	if (!CurrentAnimationData.CameraSequence)
	{
		UE_LOG(LogTemp, Error, TEXT("No camera sequence specified"));
		return;
	}

	ALevelSequenceActor* SequenceActor = nullptr;
	SequencePlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(
		GetWorld(),
		CurrentAnimationData.CameraSequence,
		FMovieSceneSequencePlaybackSettings(),
		SequenceActor
	);

	if (SequencePlayer)
	{
		SequencePlayer->OnFinished.AddDynamic(this, &UOldManCameraAnimationComponent::OnSequenceFinished);
		SequencePlayer->Play();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create level sequence player"));
	}
}

void UOldManCameraAnimationComponent::StopLevelSequence()
{
	if (SequencePlayer)
	{
		SequencePlayer->Stop();
		SequencePlayer = nullptr;
	}
}

void UOldManCameraAnimationComponent::OnSequenceFinished()
{
	// 序列动画结束时，根据停止条件处理
	if (CurrentAnimationData.StopCondition == ECameraAnimationStopCondition::OnAnimationEnd)
	{
		StartExitBlend();
	}
}

bool UOldManCameraAnimationComponent::CheckStopConditions(float DeltaTime)
{
	if (!bIsAnimationPlaying) return false;

	// 根据停止条件检查是否需要停止
	switch (CurrentAnimationData.StopCondition)
	{
	case ECameraAnimationStopCondition::ManualStop:
		// 手动停止，不自动停止
		break;

	case ECameraAnimationStopCondition::OnAnimationEnd:
		// 对于序列动画，在OnSequenceFinished中处理
		// 对于非序列动画，这里不处理
		break;

	case ECameraAnimationStopCondition::DurationBased:
		// 基于持续时间的停止条件
		if (AnimationElapsedTime >= CurrentAnimationData.Duration)
		{
			StartExitBlend();
			return true;
		}
		break;

	case ECameraAnimationStopCondition::OnExitTrigger:
	case ECameraAnimationStopCondition::OnSpecificTrigger:
		// 这些由触发器处理
		break;
	}

	return false;
}

void UOldManCameraAnimationComponent::StartExitBlend()
{
	// 记录当前相机Actor的位置、旋转和FOV作为退出混合的固定起点
	if (CameraAnimationActor)
	{
		ExitStartLocation = CameraAnimationActor->GetActorLocation();
		ExitStartRotation = CameraAnimationActor->GetActorRotation();
		if (UCameraComponent* CamComp = CameraAnimationActor->GetCameraComponent())
		{
			ExitStartFOV = CamComp->FieldOfView;
		}
		else
		{
			ExitStartFOV = CurrentAnimationData.bOverrideFOV ? CurrentAnimationData.FOV : OriginalFOV;
		}
	}
	else
	{
		// 如果没有Actor（不应该发生），则使用计算出的当前位置
		ExitStartLocation = CalculateAnimatedCameraLocation();
		ExitStartRotation = CalculateAnimatedCameraRotation();
		ExitStartFOV = CurrentAnimationData.bOverrideFOV ? CurrentAnimationData.FOV : OriginalFOV;
	}

	ExitBlendTime = CurrentAnimationData.BlendOutTime > 0.0f ? CurrentAnimationData.BlendOutTime : 0.5f;
	ExitBlendElapsed = 0.0f;

	bIsBlendingOut = true;
	AnimationBlendAlpha = 1.0f; // 从完全动画状态开始退出

	UE_LOG(LogTemp, Log, TEXT("Starting exit blend from fixed start point (Loc=%s, Rot=%s) with time: %.2f seconds"),
		*ExitStartLocation.ToString(), *ExitStartRotation.ToString(), ExitBlendTime);
}

void UOldManCameraAnimationComponent::HandleMouseExposure(bool bExpose)
{
	if (!CameraComponent || !Character) return;

	if (bExpose)
	{
		// 切换到鼠标光标模式
		CameraComponent->SetMouseCursorMode();
		AOldManPersonPlayerController* PlayerController = Character->GetOldManController();
		if (PlayerController)
		{
			// 允许鼠标点击输入，但禁止其他输入
			PlayerController->SetInputMode(FInputModeGameAndUI());
			PlayerController->bShowMouseCursor = true;
			PlayerController->SetInputEnabled(!CurrentAnimationData.bDisablePlayerInput);
			//PlayerController->SetGamepadCursorMode(true);
		}
	}
	else
	{
		// 恢复到原来的相机模式
		if (CurrentAnimationData.bDisablePlayerInput)
		{
			AOldManPersonPlayerController* PlayerController = Character->GetOldManController();
			if (PlayerController)
			{
				PlayerController->SetInputMode(FInputModeGameOnly());
				PlayerController->bShowMouseCursor = false;
				PlayerController->SetInputEnabled(false);
			}
		}
		else
		{
			// 恢复玩家输入设置
			AOldManPersonPlayerController* PlayerController = Character->GetOldManController();
			if (PlayerController)
			{
				PlayerController->SetInputMode(FInputModeGameOnly());
				PlayerController->bShowMouseCursor = false;
				PlayerController->SetInputEnabled(true);
			}
		}

		// 恢复相机模式到第三人称或重力模式，根据角色的重力状态
		if (Character->bIsOnSlope)
		{
			CameraComponent->SetPersonInSlopeMode();
		}
		else
		{
			CameraComponent->SetThirdPersonMode();
		}
	}
}