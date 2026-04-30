// Fill out your copyright notice in the Description page of Project Settings.

#include "Boss/OldManBossHead.h"
#include "Kismet/KismetMathLibrary.h"


// Sets default values
AOldManBossHead::AOldManBossHead()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

//根据记录数据更新老人头位置  没有平滑直接到位
void AOldManBossHead::UpdateData()
{
	if (CanRunning)
	{
		if (IsLeftEyebrowOpen)
		{
			LeftEyebrow->SetActorLocation(LeftEyebrowOpenPos);
			LeftEyebrow->SetActorRotation(LeftEyebrowOpenRot);
		}
		else
		{
			LeftEyebrow->SetActorLocation(LeftEyebrowInitialPos);
			LeftEyebrow->SetActorRotation(LeftEyebrowInitialRot);
		}
		if (IsRightEyebrowOpen)
		{
			RightEyebrow->SetActorLocation(RightEyebrowOpenPos);
			RightEyebrow->SetActorRotation(RightEyebrowOpenRot);
		}
		else
		{
			RightEyebrow->SetActorLocation(RightEyebrowInitialPos);
			RightEyebrow->SetActorRotation(RightEyebrowInitialRot);
		}
		LeftEarDraged(LeftEarProgress);
		RightEarDraged(RightEarProgress);
		if (IsChinOpen)
		{
			Chin->SetActorLocation(ChinOpenPos);
			Chin->SetActorRotation(ChinOpenRot);
			ShangBaL->SetActorLocation(ShangBaLOpenPos);
			ShangBaL->SetActorRotation(ShangBaLOpenRot);
			ShangBaMid->SetActorLocation(ShangBaMidOpenPos);
			ShangBaMid->SetActorRotation(ShangBaMidOpenRot);
			ShangBaR->SetActorLocation(ShangBaROpenPos);
			ShangBaR->SetActorRotation(ShangBaROpenRot);
		}
		else
		{
			Chin->SetActorLocation(ChinInitialPos);
			Chin->SetActorRotation(ChinInitialRot);
			ShangBaL->SetActorLocation(ShangBaLInitialPos);
			ShangBaL->SetActorRotation(ShangBaLInitialRot);
			ShangBaMid->SetActorLocation(ShangBaMidInitialPos);
			ShangBaMid->SetActorRotation(ShangBaMidInitialRot);
			ShangBaR->SetActorLocation(ShangBaRInitialPos);
			ShangBaR->SetActorRotation(ShangBaRInitialRot);
		}
	}
}

void AOldManBossHead::UpdateInitPodRot()
{
	if (CanRunning)
	{
		LeftEyebrowInitialPos = LeftEyebrow->GetActorLocation();
		LeftEyebrowInitialRot = LeftEyebrow->GetActorRotation();
		RightEyebrowInitialPos = RightEyebrow->GetActorLocation();
		RightEyebrowInitialRot = RightEyebrow->GetActorRotation();
		ChinInitialPos = Chin->GetActorLocation();
		ChinInitialRot = Chin->GetActorRotation();
		ShangBaLInitialPos = ShangBaL->GetActorLocation();
		ShangBaLInitialRot = ShangBaL->GetActorRotation();
		ShangBaMidInitialPos = ShangBaMid->GetActorLocation();
		ShangBaMidInitialRot = ShangBaMid->GetActorRotation();
		ShangBaRInitialPos = ShangBaR->GetActorLocation();
		ShangBaRInitialRot = ShangBaR->GetActorRotation();
		LeftEarInitialRot = LeftEar->GetActorRotation();
		RightEarInitialRot = RightEar->GetActorRotation();
	}

}

//设置指定部位激活part
void AOldManBossHead::SetPartActive(ECurOperationType target, bool Active)
{
	if (CanRunning)
	{
		switch (target)
		{
		case ECurOperationType::LeftEyebrow:
			IsLeftEyebrowActive = Active;
			break;
		case ECurOperationType::RightEyebrow:
			IsRightEyebrowActive = Active;
			break;
		case ECurOperationType::Chin:
			IsChinActive = Active;
			break;
		case ECurOperationType::TurnHeadLeft:
			IsLeftEarActive = Active;
			break;
		case ECurOperationType::TurnHeadRight:
			IsRightEarActive = Active;
			break;
		case ECurOperationType::Eyebrow:
			IsLeftEyebrowActive = Active;
			IsRightEyebrowActive = Active;
			break;
		case ECurOperationType::None:
			break;
		}
	}
}
//设置所有部位激活part
void AOldManBossHead::SetAllPartActive(bool Active)
{
	if (CanRunning)
	{
		IsLeftEyebrowActive = Active;
		IsRightEyebrowActive = Active;
		IsChinActive = Active;
		IsLeftEarActive = Active;
		IsRightEarActive = Active;
	}
}

void AOldManBossHead::SetAllPartBack()
{
	AllPartsMovingEnd = true;
	if (IsLeftEyebrowOpen)LeftEyebrowClose();
	if (IsRightEyebrowOpen)RightEyebrowClose();
	if (IsChinOpen)ChinClose();
	if (LeftEarProgress != 0)LeftEarBack();
	if (RightEarProgress != 0)RightEarBack();
}

void AOldManBossHead::LeftEyebrowOpen()
{
	if (CanRunning && IsLeftEyebrowActive)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsLeftEyebrowMoving = true;
		IsLeftEyebrowOpen = false;
	}
}

void AOldManBossHead::LeftEyebrowClose()
{
	if (CanRunning)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsLeftEyebrowMoving = true;
		IsLeftEyebrowOpen = true;
	}

}

void AOldManBossHead::LeftEyebrowBlink()
{
	//等待高亮事件
	LeftEyebrowBlinkInBP();
}

void AOldManBossHead::RightEyebrowOpen()
{
	if (CanRunning && IsRightEyebrowActive)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsRightEyebrowMoving = true;
		IsRightEyebrowOpen = false;
	}
}

void AOldManBossHead::RightEyebrowClose()
{
	if (CanRunning)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsRightEyebrowMoving = true;
		IsRightEyebrowOpen = true;
	}

}

void AOldManBossHead::RightEyebrowBlink()
{
	//等待高亮事件
	RightEyebrowBlinkInBP();
}

void AOldManBossHead::ChinOpen()
{
	if (CanRunning && IsChinActive)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsChinAllMoving = true;
		IsChinMoving = true;
		IsShangBaLMoving = true;
		IsShangBaMidMoving = true;
		IsShangBaRMoving = true;
		IsChinOpen = false;
	}

}

void AOldManBossHead::ChinClose()
{
	if (CanRunning)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsChinAllMoving = true;
		IsChinMoving = true;
		IsShangBaLMoving = true;
		IsShangBaMidMoving = true;
		IsShangBaRMoving = true;
		IsChinOpen = true;
	}

}

void AOldManBossHead::ChinBlink()
{
	//等待高亮事件
	ChinBlinkInBP();
}

void AOldManBossHead::LeftEarDragedAdd(float Progress)
{
	if (CanRunning && IsLeftEarActive)
	{
		LeftEarProgress += Progress;
		if (LeftEarProgress >= 1)
		{
			LeftEarProgress = 1;
			LeftEarCompelete();
		}
		LeftEarDraged(LeftEarProgress);
	}
}

void AOldManBossHead::LeftEarDraged(float curProgress)
{
	if (CanRunning && IsLeftEarActive)
	{
		LeftEarProgress = FMath::Clamp(curProgress, 0.0f, 1.0f);
		// 转换为四元数以获得更好的插值效果（自动处理最短路径）
		FQuat InitialQuat = GetActorRotation().Quaternion();
		FQuat TargetQuat = LeftEarMax.Quaternion();
		// 球面线性插值 (Slerp) 产生平滑且最短路径的旋转
		FQuat InterpolatedQuat = FQuat::Slerp(InitialQuat, TargetQuat, LeftEarProgress);

		// 应用旋转
		SetActorRotation(InterpolatedQuat);
		JudgeLeftEarRight();
	}
}

void AOldManBossHead::LeftEarBack()
{
	if (CanRunning)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsLeftEarBackMoving = true;
		LeftEarBackCompelete = false;
	}

}

void AOldManBossHead::LeftEarBlink()
{
	//等待高亮事件
	LeftEarBlinkInBP();
}

void AOldManBossHead::JudgeLeftEarRight()
{
	float fTolerance = FMath::Max3(LeftEarDetectionRange.Pitch, LeftEarDetectionRange.Yaw, LeftEarDetectionRange.Roll);
	if (GetActorRotation().Equals(LeftEarMax, fTolerance))
	{
		IsLeftEarRight = true;
	}
	else IsLeftEarRight = false;
	if (IsLeftEarRight)GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("BossHead_LeftEar_True"));
}

void AOldManBossHead::RightEarDragedAdd(float Progress)
{
	if (CanRunning && IsRightEarActive)
	{
		RightEarProgress += Progress;
		if (RightEarProgress >= 1)
		{
			RightEarProgress = 1;
			RightEarCompelete();
		}
		RightEarDraged(RightEarProgress);
	}
}

void AOldManBossHead::RightEarDraged(float curProgress)
{
	if (CanRunning && IsRightEarActive)
	{
		RightEarProgress = FMath::Clamp(curProgress, 0.0f, 1.0f);
		// 转换为四元数以获得更好的插值效果（自动处理最短路径）
		FQuat InitialQuat = GetActorRotation().Quaternion();
		FQuat TargetQuat = RightEarMax.Quaternion();
		// 球面线性插值 (Slerp) 产生平滑且最短路径的旋转
		FQuat InterpolatedQuat = FQuat::Slerp(InitialQuat, TargetQuat, RightEarProgress);
		// 应用旋转
		SetActorRotation(InterpolatedQuat);
		JudgeRightEarRight();
	}
}

void AOldManBossHead::RightEarBack()
{
	if (CanRunning)
	{
		PrimaryActorTick.bStartWithTickEnabled = true;
		SetActorTickEnabled(true); // 开启Tick
		IsRightEarBackMoving = true;
		RightEarBackCompelete = false;
	}

}

void AOldManBossHead::RightEarBlink()
{
	//等待高亮事件
	RightEarBlinkInBP();
}

void AOldManBossHead::JudgeRightEarRight()
{
	float fTolerance = FMath::Max3(RightEarDetectionRange.Pitch, RightEarDetectionRange.Yaw, RightEarDetectionRange.Roll);
	FRotator now = GetActorRotation();
	if (now.Equals(RightEarMax, fTolerance))
	{
		IsRightEarRight = true;
	}
	else IsRightEarRight = false;
	if (IsRightEarRight)GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("BossHead_RightEar_True"));
}

// Called when the game starts or when spawned
void AOldManBossHead::BeginPlay()
{
	Super::BeginPlay();
	//初始化
	CanRunning = true;
	if (LeftEyebrow)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_左眉毛不存在"));
		CanRunning = false;
	}
	if (RightEyebrow)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_右眉毛不存在"));
		CanRunning = false;
	}
	if (Chin)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_下巴不存在"));
		CanRunning = false;
	}
	if (ShangBaL)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_上巴左不存在"));
		CanRunning = false;
	}
	if (ShangBaMid)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_上巴中不存在"));
		CanRunning = false;
	}
	if (ShangBaR)
	{
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_上巴右不存在"));
		CanRunning = false;
	}
	if (!LeftEar)
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_左耳朵不存在"));
		CanRunning = false;
	}
	if (!RightEar)
	{
		UE_LOG(LogTemp, Error, TEXT("BossHead_右耳朵不存在"));
		CanRunning = false;
	}
	
	IsLeftEyebrowOpen = false;
	IsRightEyebrowOpen = false;
	IsChinOpen = false;
	LeftEarProgress = 0;
	JudgeLeftEarRight();
	RightEarProgress = 0;
	JudgeRightEarRight();

	SetAllPartActive(false);
	UpdateInitPodRot();
	UpdateData();
}

// Called every frame
void AOldManBossHead::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!IsRightEarBackMoving && !IsLeftEarBackMoving && !IsChinAllMoving && !IsRightEyebrowMoving && !IsLeftEyebrowMoving)
	{
		SetActorTickEnabled(true); // 停止Tick
		AllPartsMovingEnd = true;
		return;
	}
	else
	{
		AllPartsMovingEnd = false;
	}

	//左眉毛
	if (IsLeftEyebrowMoving)
	{
		// 获取当前位置和旋转
		FVector CurrentLELocation = LeftEyebrow->GetActorLocation();
		FRotator CurrentLERotation = LeftEyebrow->GetActorRotation();
		// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
		FVector NewLELocation = UKismetMathLibrary::VInterpTo(CurrentLELocation, IsLeftEyebrowOpen? LeftEyebrowInitialPos: LeftEyebrowOpenPos, DeltaTime, AllSpeed);
		// 计算新旋转 (RInterpTo 内部使用最短路径插值)
		FRotator NewLERotation = UKismetMathLibrary::RInterpTo(CurrentLERotation, IsLeftEyebrowOpen ? LeftEyebrowInitialRot : LeftEyebrowOpenRot, DeltaTime, AllSpeed);
		// 设置新变换
		LeftEyebrow->SetActorLocationAndRotation(NewLELocation, NewLERotation);
		// 检查是否足够接近目标，是则直接设为目标值并停止Tick
		float Distance = FVector::Dist(NewLELocation, IsLeftEyebrowOpen ? LeftEyebrowInitialPos : LeftEyebrowOpenPos);
		FQuat QuatA = NewLERotation.Quaternion();
		FQuat QuatB = (IsLeftEyebrowOpen ? LeftEyebrowInitialRot : LeftEyebrowOpenRot).Quaternion();
		float AngleDiffRadians = QuatA.AngularDistance(QuatB);

		if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
		{
			// 直接设置到精确目标
			LeftEyebrow->SetActorLocationAndRotation(IsLeftEyebrowOpen ? LeftEyebrowInitialPos : LeftEyebrowOpenPos, IsLeftEyebrowOpen ? LeftEyebrowInitialRot : LeftEyebrowOpenRot, false, nullptr, ETeleportType::None);
			IsLeftEyebrowMoving = false;
			
			IsLeftEyebrowOpen = !IsLeftEyebrowOpen;
		}
	}

	//右眉毛
	if (IsRightEyebrowMoving)
	{
		// 获取当前位置和旋转
		FVector CurrentRELocation = RightEyebrow->GetActorLocation();
		FRotator CurrentRERotation = RightEyebrow->GetActorRotation();
		// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
		FVector NewRELocation = UKismetMathLibrary::VInterpTo(CurrentRELocation, IsRightEyebrowOpen ? RightEyebrowInitialPos : RightEyebrowOpenPos, DeltaTime, AllSpeed);
		// 计算新旋转 (RInterpTo 内部使用最短路径插值)
		FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, IsRightEyebrowOpen ? RightEyebrowInitialRot : RightEyebrowOpenRot, DeltaTime, AllSpeed);
		// 设置新变换
		RightEyebrow->SetActorLocationAndRotation(NewRELocation, NewRERotation, false, nullptr, ETeleportType::None);
		// 检查是否足够接近目标，是则直接设为目标值并停止Tick
		float Distance = FVector::Dist(NewRELocation, IsRightEyebrowOpen ? RightEyebrowInitialPos : RightEyebrowOpenPos);
		FQuat QuatA = NewRERotation.Quaternion();
		FQuat QuatB = (IsRightEyebrowOpen ? RightEyebrowInitialRot : RightEyebrowOpenRot).Quaternion();
		float AngleDiffRadians = QuatA.AngularDistance(QuatB);

		if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
		{
			// 直接设置到精确目标
			RightEyebrow->SetActorLocationAndRotation(IsRightEyebrowOpen ? RightEyebrowInitialPos : RightEyebrowOpenPos, IsRightEyebrowOpen ? RightEyebrowInitialRot : RightEyebrowOpenRot, false, nullptr, ETeleportType::None);
			IsRightEyebrowMoving = false;
			IsRightEyebrowOpen = !IsRightEyebrowOpen;
		}
	}

	//下巴
	if (IsChinAllMoving)
	{
		//下巴
		if (IsChinMoving)
		{
			// 获取当前位置和旋转
			FVector CurrentRELocation = Chin->GetActorLocation();
			FRotator CurrentRERotation = Chin->GetActorRotation();
			// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
			FVector NewRELocation = UKismetMathLibrary::VInterpTo(CurrentRELocation, IsChinOpen ? ChinInitialPos : ChinOpenPos, DeltaTime, AllSpeed);
			// 计算新旋转 (RInterpTo 内部使用最短路径插值)
			FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, IsChinOpen ? ChinInitialRot : ChinOpenRot, DeltaTime, AllSpeed);
			// 设置新变换
			Chin->SetActorLocationAndRotation(NewRELocation, NewRERotation, false, nullptr, ETeleportType::None);
			// 检查是否足够接近目标，是则直接设为目标值并停止Tick
			float Distance = FVector::Dist(NewRELocation, IsChinOpen ? ChinInitialPos : ChinOpenPos);
			FQuat QuatA = NewRERotation.Quaternion();
			FQuat QuatB = (IsChinOpen ? ChinInitialRot : ChinOpenRot).Quaternion();
			float AngleDiffRadians = QuatA.AngularDistance(QuatB);

			if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
			{
				// 直接设置到精确目标
				Chin->SetActorLocationAndRotation(IsChinOpen ? ChinInitialPos : ChinOpenPos, IsChinOpen ? ChinInitialRot : ChinOpenRot, false, nullptr, ETeleportType::None);
				IsChinMoving = false;
			}
		}

		if (IsShangBaLMoving)
		{
			// 获取当前位置和旋转
			FVector CurrentRELocation = ShangBaL->GetActorLocation();
			FRotator CurrentRERotation = ShangBaL->GetActorRotation();
			// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
			FVector NewRELocation = UKismetMathLibrary::VInterpTo(CurrentRELocation, IsChinOpen ? ShangBaLInitialPos : ShangBaLOpenPos, DeltaTime, AllSpeed);
			// 计算新旋转 (RInterpTo 内部使用最短路径插值)
			FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, IsChinOpen ? ShangBaLInitialRot : ShangBaLOpenRot, DeltaTime, AllSpeed);
			// 设置新变换
			ShangBaL->SetActorLocationAndRotation(NewRELocation, NewRERotation, false, nullptr, ETeleportType::None);
			// 检查是否足够接近目标，是则直接设为目标值并停止Tick
			float Distance = FVector::Dist(NewRELocation, IsChinOpen ? ShangBaLInitialPos : ShangBaLOpenPos);
			FQuat QuatA = NewRERotation.Quaternion();
			FQuat QuatB = (IsChinOpen ? ShangBaLInitialRot : ShangBaLOpenRot).Quaternion();
			float AngleDiffRadians = QuatA.AngularDistance(QuatB);

			if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
			{
				// 直接设置到精确目标
				ShangBaL->SetActorLocationAndRotation(IsChinOpen ? ShangBaLInitialPos : ShangBaLOpenPos, IsChinOpen ? ShangBaLInitialRot : ShangBaLOpenRot, false, nullptr, ETeleportType::None);
				IsShangBaLMoving = false;
			}

		}
		if (IsShangBaMidMoving)
		{
			// 获取当前位置和旋转
			FVector CurrentRELocation = ShangBaMid->GetActorLocation();
			FRotator CurrentRERotation = ShangBaMid->GetActorRotation();
			// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
			FVector NewRELocation = UKismetMathLibrary::VInterpTo(CurrentRELocation, IsChinOpen ? ShangBaMidInitialPos : ShangBaMidOpenPos, DeltaTime, AllSpeed);
			// 计算新旋转 (RInterpTo 内部使用最短路径插值)
			FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, IsChinOpen ? ShangBaMidInitialRot : ShangBaMidOpenRot, DeltaTime, AllSpeed);
			// 设置新变换
			ShangBaMid->SetActorLocationAndRotation(NewRELocation, NewRERotation, false, nullptr, ETeleportType::None);
			// 检查是否足够接近目标，是则直接设为目标值并停止Tick
			float Distance = FVector::Dist(NewRELocation, IsChinOpen ? ShangBaMidInitialPos : ShangBaMidOpenPos);
			FQuat QuatA = NewRERotation.Quaternion();
			FQuat QuatB = (IsChinOpen ? ShangBaMidInitialRot : ShangBaMidOpenRot).Quaternion();
			float AngleDiffRadians = QuatA.AngularDistance(QuatB);

			if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
			{
				// 直接设置到精确目标
				ShangBaMid->SetActorLocationAndRotation(IsChinOpen ? ShangBaMidInitialPos : ShangBaMidOpenPos, IsChinOpen ? ShangBaMidInitialRot : ShangBaMidOpenRot, false, nullptr, ETeleportType::None);
				IsShangBaMidMoving = false;
			}

		}
		if (IsShangBaRMoving)
		{
			// 获取当前位置和旋转
			FVector CurrentRELocation = ShangBaR->GetActorLocation();//把这个和下面哪个改成ShangBaMid会导致运行时张开嘴巴后检测无法进入下一阶段，但在嘴巴张开后打开细节面板或在577行打断点就可以正常运行？？？？？？？？？？？？？？
			FRotator CurrentRERotation = ShangBaR->GetActorRotation();
			// 计算新位置 (指数插值，DeltaTime * InterpSpeed 控制衰减速度)
			FVector NewRELocation = UKismetMathLibrary::VInterpTo(CurrentRELocation, IsChinOpen ? ShangBaRInitialPos : ShangBaROpenPos, DeltaTime, AllSpeed);
			// 计算新旋转 (RInterpTo 内部使用最短路径插值)
			FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, IsChinOpen ? ShangBaRInitialRot : ShangBaROpenRot, DeltaTime, AllSpeed);
			// 设置新变换
			ShangBaR->SetActorLocationAndRotation(NewRELocation, NewRERotation, false, nullptr, ETeleportType::None);
			// 检查是否足够接近目标，是则直接设为目标值并停止Tick
			float Distance = FVector::Dist(NewRELocation, IsChinOpen ? ShangBaRInitialPos : ShangBaROpenPos);
			FQuat QuatA = NewRERotation.Quaternion();
			FQuat QuatB = (IsChinOpen ? ShangBaRInitialRot : ShangBaROpenRot).Quaternion();
			float AngleDiffRadians = QuatA.AngularDistance(QuatB);

			if (Distance <= 0.05f && AngleDiffRadians <= 0.05f)
			{
				// 直接设置到精确目标
				ShangBaR->SetActorLocationAndRotation(IsChinOpen ? ShangBaRInitialPos : ShangBaROpenPos, IsChinOpen ? ShangBaRInitialRot : ShangBaROpenRot, false, nullptr, ETeleportType::None);
				IsShangBaRMoving = false;
			}

		}
		if (!IsChinMoving && !IsShangBaLMoving && !IsShangBaMidMoving && !IsShangBaRMoving)
		{
			IsChinAllMoving = false;
			IsChinOpen = !IsChinOpen;
		}
	}

	//左转头
	if (IsLeftEarBackMoving)
	{
		// 获取当前旋转
		FRotator CurrentRERotation = GetActorRotation();
		// 计算新旋转 (RInterpTo 内部使用最短路径插值)
		FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, LeftEarInitialRot, DeltaTime, AllEarSpeed);
		// 设置新变换
		SetActorRotation(NewRERotation);
		// 检查是否足够接近目标，是则直接设为目标值并停止Tick
		FQuat QuatA = NewRERotation.Quaternion();
		FQuat QuatB = LeftEarInitialRot.Quaternion();
		float AngleDiffRadians = QuatA.AngularDistance(QuatB);

		if (AngleDiffRadians <= 0.1f)
		{
			// 直接设置到精确目标
			SetActorRotation(LeftEarInitialRot);
			IsLeftEarBackMoving = false;
			JudgeLeftEarRight();
			LeftEarBackCompelete = true;
			LeftEarProgress = 0;
		}
	}
	//右转头
	if (IsRightEarBackMoving)
	{
		// 获取当前旋转
		FRotator CurrentRERotation = GetActorRotation();
		// 计算新旋转 (RInterpTo 内部使用最短路径插值)
		FRotator NewRERotation = UKismetMathLibrary::RInterpTo(CurrentRERotation, RightEarInitialRot, DeltaTime, AllEarSpeed);
		// 设置新变换
		SetActorRotation(NewRERotation);
		// 检查是否足够接近目标，是则直接设为目标值并停止Tick
		FQuat QuatA = NewRERotation.Quaternion();
		FQuat QuatB = RightEarInitialRot.Quaternion();
		float AngleDiffRadians = QuatA.AngularDistance(QuatB);

		if (AngleDiffRadians <= 0.2f)
		{
			// 直接设置到精确目标
			SetActorRotation(RightEarInitialRot);
			IsRightEarBackMoving = false;
			JudgeRightEarRight();
			RightEarBackCompelete = true;
			RightEarProgress = 0;
		}
	}

}

