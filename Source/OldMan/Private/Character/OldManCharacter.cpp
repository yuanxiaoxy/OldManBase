#include "Character/OldManCharacter.h"
#include "Character/OldManPersonPlayerController.h"
#include "StateMachine/StateMachineBase.h"
#include "Character/States/OldManIdleState.h"
#include "Character/States/OldManWalkingState.h"
#include "Character/States/OldManRunningState.h"
#include "Character/States/OldManJumpingState.h"
#include "Character/States/OldManDoubleJumpingState.h"
#include "Character/States/OldManFallingState.h"
#include "Character/States/OldManLandState.h"
#include "Character/States/OldManAttackingState.h"
#include "Character/States/OldManDeadState.h"
#include "Components/InputComponent.h"
#include "Components/BoxComponent.h"
#include "GlobalTagName.h"
#include "GlobalEventName.h"

AOldManCharacter::AOldManCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<UOldManMovementComponent>(AOldManCharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    //创建移动组件
    OldManMovementComponent = Cast<UOldManMovementComponent>(Super::GetMovementComponent());

    // 创建碰撞组件
    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    InteractionBox->SetupAttachment(RootComponent);
    InteractionBox->SetCollisionProfileName(TEXT("Player"));

    bulletFirePos = CreateDefaultSubobject<USceneComponent>(TEXT("bulletFirePosition"));
    bulletFirePos->SetupAttachment(GetMesh());

    // 创建弹簧臂组件
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = CameraDistance;
    CameraBoom->SocketOffset = CameraOffset;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->bEnableCameraRotationLag = true;
    CameraBoom->CameraLagSpeed = 10.0f;
    CameraBoom->CameraRotationLagSpeed = 10.0f;
    CameraBoom->bDoCollisionTest = true;

    // 创建跟随相机组件
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // 创建相机控制组件
    CameraComponent = CreateDefaultSubobject<UOldManCameraComponent>(TEXT("CameraComponent"));

    // 确保角色不自动朝向移动方向，由我们手动控制
    bUseControllerRotationYaw = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bOrientRotationToMovement = false;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    }

    Tags.Add(UGlobalTagName::Tag_Player);
}

void AOldManCharacter::BeginPlay()
{
    Super::BeginPlay();
    InitializeParam();
    InitializeCameraComponent();
    InitializeStateMachine();
    InitializeEvent();

    EnableCustomGravity(true);
}

void AOldManCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 更新状态机
    if (StateMachine && StateMachine->IsRunning())
    {
        StateMachine->Update(DeltaTime);
    }

    // 持续射线检测更新重力
    if (bCustomGravityEnabled)
    {
        RaycastTimer += DeltaTime;
        if (RaycastTimer >= 0.1f) // 每0.1秒检测一次
        {
            UpdateGravityByRaycast();
            RaycastTimer = 0.0f;
        }
    }
}

#pragma region Control Param
void AOldManCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    EMovementMode NewMovementMode = GetCharacterMovement()->MovementMode;

    // 检测从下落状态切换到行走状态（表示落地）
    if (PrevMovementMode == MOVE_Falling && NewMovementMode == MOVE_Walking)
    {
        LastLandingTime = GetWorld()->GetTimeSeconds();
        bWasFalling = false;

        UE_LOG(LogTemp, Log, TEXT("Character landed successfully"));
    }
    // 检测开始下落
    else if (NewMovementMode == MOVE_Falling)
    {
        bWasFalling = true;
        UE_LOG(LogTemp, Log, TEXT("Character started falling"));
    }
}

bool AOldManCharacter::CanJumpInternal_Implementation() const
{
    return true;
}

AOldManPersonPlayerController* AOldManCharacter::GetOldManController()
{
    if (!OldManController)
    {
        OldManController = Cast<AOldManPersonPlayerController>(GetController());
    }
    return OldManController;
}

void AOldManCharacter::SetMovementInput(FVector inputDir)
{
    MovementInputVector = inputDir;
}

void AOldManCharacter::SetJumpInput(bool bJumping)
{
    bHasJumpInput = bJumping;
}

void AOldManCharacter::SetRunning(bool bRunning)
{
    bIsRunning = bRunning;
}

void AOldManCharacter::ChangeSlopeState(bool slopeState)
{
    bIsOnSlope = slopeState;
}

void AOldManCharacter::UpdateGravityByRaycast()
{
    if (OldManMovementComponent && bCustomGravityEnabled)
    {
        OldManMovementComponent->SetGravityByRaycast(false);
    }
}

void AOldManCharacter::EnableCustomGravity(bool bEnable)
{
    bCustomGravityEnabled = bEnable;
    if (!bEnable && OldManMovementComponent)
    {
        OldManMovementComponent->ResetGravityDirection(true);
    }
}

bool AOldManCharacter::IsUsingCustomGravity() const
{
    return OldManMovementComponent && OldManMovementComponent->bUseCustomGravity;
}

FVector AOldManCharacter::GetCurrentGravityDirection() const
{
    if (OldManMovementComponent)
    {
        return OldManMovementComponent->CurrentGravityDirection;
    }
    return FVector(0, 0, -1);
}

// 修改 UpdateCharacterRotation 以兼容自定义重力：
void AOldManCharacter::UpdateCharacterRotation(float DeltaTime, const FVector& DesiredDirection)
{
    if (DesiredDirection.IsNearlyZero())
        return;

    // 如果使用自定义重力，让重力系统处理角色朝向
    if (OldManMovementComponent && OldManMovementComponent->bUseCustomGravity)
    {
        // 在自定义重力下，我们只调整Yaw，让重力系统处理角色的整体朝向
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = DesiredDirection.Rotation();

        // 只插值Yaw
        float NewYaw = FMath::RInterpTo(
            FRotator(0, CurrentRotation.Yaw, 0),
            FRotator(0, TargetRotation.Yaw, 0),
            DeltaTime,
            CharacterAttributes ? CharacterAttributes->RotationBlendInterpSpeed : 8.0f
        ).Yaw;

        // 保持当前的Pitch和Roll（由重力系统控制）
        FRotator NewRotation = CurrentRotation;
        NewRotation.Yaw = NewYaw;

        SetActorRotation(NewRotation);
    }
    else
    {
        // 正常重力下的旋转逻辑
        FRotator CurrentRotation = GetActorRotation();
        FRotator TargetRotation = DesiredDirection.Rotation();

        // 计算旋转差异，避免小角度的抖动
        float YawDifference = FMath::Abs(CurrentRotation.Yaw - TargetRotation.Yaw);
        if (YawDifference > 1.0f)
        {
            FRotator NewRotation = FMath::RInterpTo(
                CurrentRotation,
                TargetRotation,
                DeltaTime,
                CharacterAttributes ? CharacterAttributes->RotationBlendInterpSpeed : 8.0f
            );
            SetActorRotation(FRotator(0.0f, NewRotation.Yaw, 0.0f));
        }
    }
}

FVector AOldManCharacter::GetMovementDirectionFromCamera() const
{
    if (CameraComponent && !MovementInputVector.IsNearlyZero())
    {
        FRotator CameraRotation = CameraComponent->GetCameraRotation();
        CameraRotation.Pitch = 0.0f;
        CameraRotation.Roll = 0.0f;
        return CameraRotation.RotateVector(MovementInputVector);
    }
    return MovementInputVector;
}

// ========== 相机控制函数 ==========

void AOldManCharacter::SetCameraDistance(float Distance)
{
    if (CameraComponent)
    {
        CameraComponent->SetCameraDistance(Distance);
    }
}

void AOldManCharacter::SetCameraOffset(const FVector& Offset)
{
    if (CameraComponent)
    {
        CameraComponent->SetCameraOffset(Offset);
    }
}

void AOldManCharacter::SetThirdPersonMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetThirdPersonMode();
    }
}

void AOldManCharacter::SetFirstPersonMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetFirstPersonMode();
    }
}

void AOldManCharacter::SetFreeLookMode()
{
    if (CameraComponent)
    {
        CameraComponent->SetFreeLookMode();
    }
}

void AOldManCharacter::ShakeCamera(float Intensity, float Duration)
{
    if (CameraComponent)
    {
        CameraComponent->ShakeCamera(Intensity, Duration);
    }
}

bool AOldManCharacter::IsMoving() const
{
    return GetVelocity().SizeSquared() > 0.1f;
}

bool AOldManCharacter::IsFalling() const
{
    // 使用更可靠的检测方法
    if (!GetCharacterMovement())
        return false;

    // 如果移动组件说在下落，并且没有刚落地，则认为在下落
    bool bMovementFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 防止刚落地时仍然返回true
    if (!bMovementFalling && (CurrentTime - LastLandingTime < 0.1f))
    {
        return false; // 刚落地，不算下落
    }

    return bMovementFalling;
}

bool AOldManCharacter::CanDoubleJump() const
{
    //判断是否进入过
    return !hasIntoDoubleJump;
}

bool AOldManCharacter::CanAttack() const
{
    if (!IsAlive()) return false;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= (CharacterAttributes ? CharacterAttributes->AttackCooldown : 1.0f);
}

bool AOldManCharacter::HasMovementInput() const
{
    return !MovementInputVector.IsNearlyZero();
}

bool AOldManCharacter::IsActuallyGrounded() const
{
    if (!GetCharacterMovement())
        return false;

    // 使用多种条件判断是否真的在地面
    bool bIsOnGround = GetCharacterMovement()->IsMovingOnGround();
    bool bIsFalling = GetCharacterMovement()->IsFalling();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // 如果移动组件说在地面，并且最近有落地事件，则认为真的在地面
    return bIsOnGround && !bIsFalling && (CurrentTime - LastLandingTime < 0.5f);
}

float AOldManCharacter::GetTimeSinceLastLanding() const
{
    return GetWorld()->GetTimeSeconds() - LastLandingTime;
}

void AOldManCharacter::PrintMovementState() const
{
    if (!GetCharacterMovement()) return;

    FString MovementState;
    switch (GetCharacterMovement()->MovementMode)
    {
    case MOVE_None: MovementState = "None"; break;
    case MOVE_Walking: MovementState = "Walking"; break;
    case MOVE_NavWalking: MovementState = "NavWalking"; break;
    case MOVE_Falling: MovementState = "Falling"; break;
    case MOVE_Swimming: MovementState = "Swimming"; break;
    case MOVE_Flying: MovementState = "Flying"; break;
    case MOVE_Custom: MovementState = "Custom"; break;
    default: MovementState = "Unknown"; break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Movement State: %s, IsFalling: %d, IsActuallyGrounded: %d"),
        *MovementState,
        GetCharacterMovement()->IsFalling(),
        IsActuallyGrounded());
}

void AOldManCharacter::PerformAttackDetection()
{
    if (!CharacterAttributes) return;

    // 攻击检测逻辑
    FVector StartLocation = GetActorLocation();
    FVector ForwardVector = GetActorForwardVector();
    FVector EndLocation = StartLocation + ForwardVector * CharacterAttributes->AttackRange;

    // 球形检测
    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(50.0f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        Params
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            AActor* HitActor = Hit.GetActor();
            if (HitActor && HitActor != this)
            {
                // 应用伤害或触发事件
                OnAttackHit(HitActor);
                UE_LOG(LogTemp, Log, TEXT("Hit actor: %s"), *HitActor->GetName());
            }
        }
    }

    LastAttackTime = GetWorld()->GetTimeSeconds();
}

void AOldManCharacter::SetupCharacterMesh(USkeletalMesh* NewMesh, UClass* NewAnimClass)
{
    if (NewMesh)
    {
        GetMesh()->SetSkeletalMesh(NewMesh);
    }

    if (NewAnimClass)
    {
        GetMesh()->SetAnimInstanceClass(NewAnimClass);
    }
}

void AOldManCharacter::DectedActors()
{
    TArray<AActor*> OutActors;
    TArray<float> OutDistances;
    TArray<float> OutAngles;
    CameraComponent->GetActorsInCone(CharacterAttributes->OldManDetectionData, UGlobalTagName::Tag_BeDetcedItem, OutActors, OutDistances, OutAngles);

    if (OutActors.Num() < 1 || OutActors.Num() != OutDistances.Num() || OutActors.Num() != OutAngles.Num())
    {
        return;
    }

    int actorIndex = 0;
    float distance = 10000.0f;

    AActor* finalActor = nullptr;
    for (int i = 0; i < OutActors.Num(); i++)
    {
        if (distance > OutDistances[i])
        {
            distance = OutDistances[i];
            actorIndex = i;
        }
        UE_LOG(LogTemp, Display, TEXT("%s"), *(OutActors[i]->GetFName().ToString()));
    }

    curAimAttackItem = Cast<AOldManCanBeAttackItemBase>(OutActors[actorIndex]);
    if (curAimAttackItem)
    {
        if (!firstKindBullet) return;

        FireBullet();
    }

    curAimAttackItem = nullptr;
}

void AOldManCharacter::InitializeParam()
{
    // 初始化变量
    bIsRunning = false;
    hasIntoDoubleJump = false;
    LastAttackTime = 0.0f;
    MovementInputVector = FVector::ZeroVector;
    bHasJumpInput = false;
    bHasAttackInput = false;
    bInCanPullState = true;

    // 重力系统设置
    bCustomGravityEnabled = true; // 默认启用自定义重力
    RaycastTimer = 0.0f;

    // 落地检测改进
    LastLandingTime = 0.0f;
    bWasFalling = false;
}

void AOldManCharacter::InitializeStateMachine()
{
    UStateMachineManager* StateMachineManager = UStateMachineManager::GetStateMachineManager();
    if (StateMachineManager)
    {
        StateMachine = StateMachineManager->CreateStateMachine(this, true);
        StateMachine->InitializeWithState(UOldManIdleState::StaticClass(), this);
    }
}

void AOldManCharacter::InitializeCameraComponent()
{
    if (CameraComponent && CameraBoom && FollowCamera)
    {
        CameraComponent->InitializeCameraComponents(CameraBoom, FollowCamera, CharacterAttributes->OldManCameraData);
        CameraComponent->SetCameraTarget(this);
    }
}

void AOldManCharacter::InitializeEvent()
{
    UMyEventManager::GetInstance()->RegisterCppEvent<AOldManCharacter, bool>(UGlobalEventName::Key_Player_OnChangeGrivity, this, &AOldManCharacter::ChangeSlopeState);
}
#pragma endregion

#pragma region Item Fun
void AOldManCharacter::FireBullet()
{
    // 生成子弹
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    AOldManBulletBase* Bullet = GetWorld()->SpawnActor<AOldManBulletBase>(firstKindBullet,
        bulletFirePos->GetComponentLocation(), bulletFirePos->GetComponentRotation(), SpawnParams);

    if (Bullet)
    {
        FVector bulletDir = curAimAttackItem->GetActorLocation() - bulletFirePos->GetComponentLocation();
        Bullet->InitializeBullet(bulletDir.GetSafeNormal(), curAimAttackItem);
    }
}

void AOldManCharacter::SetPullItemState(bool bPulling)
{
    bInCanPullState = bPulling;
}

//使用射线与Tag判断当前是否有可拖动物品能控制
void AOldManCharacter::StartRightMousePull()
{
    if (!GetOldManController() || !GetOldManController()->PlayerCameraManager) return;

    FVector CameraLocation = GetOldManController()->PlayerCameraManager->GetCameraLocation();
    FRotator CameraRotation = GetOldManController()->PlayerCameraManager->GetCameraRotation();
    FVector CameraDirection = CameraRotation.Vector();

    UE_LOG(LogTemp, Display, TEXT("Camera Raycast - Location: %s, Direction: %s"),
        *CameraLocation.ToString(), *CameraDirection.ToString());

    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(GetOldManController()->GetPawn());

    FVector TraceEnd = CameraLocation + CameraDirection * 10000.0f;

    // 调试绘制
    DrawDebugLine(GetWorld(), CameraLocation, TraceEnd, FColor::Cyan, false, 5.0f, 0, 2.0f);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_Visibility, QueryParams))
    {
        AOldManPullItemBase* HitActor = Cast<AOldManPullItemBase>(HitResult.GetActor());
        if (HitActor)
        {
            SetPullItemState(true);
            HitActor->StartDragging();
            curOldManPullItem = HitActor;

            // 绘制命中点
            DrawDebugSphere(GetWorld(), HitResult.Location, 15.0f, 12, FColor::Magenta, false, 5.0f, 0, 3.0f);
        }
    }
}

void AOldManCharacter::StopRightMousePull()
{
    SetPullItemState(false);
    if (curOldManPullItem)
    {
        curOldManPullItem->StopDragging();
        curOldManPullItem = nullptr;
    }
}

void AOldManCharacter::HandleMouseLook(FVector2D mouseDelta)
{
    // 鼠标输入
    float MouseXInput = mouseDelta.X;
    float MouseYInput = mouseDelta.Y;

    // 处理拖动
    if (curOldManPullItem && bInCanPullState)
    {
        APlayerCameraManager* CameraManager = GetOldManController()->PlayerCameraManager;
        if (CameraManager && (FMath::Abs(MouseXInput) > MinMovementThreshold || FMath::Abs(MouseYInput) > MinMovementThreshold))
        {
            FVector CameraLocation = CameraManager->GetCameraLocation();
            FRotator CameraRotation = CameraManager->GetCameraRotation();

            // 获取相机的方向向量
            FVector CameraForward = CameraRotation.Vector();
            FVector CameraRight = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Y);
            FVector CameraUp = FRotationMatrix(CameraRotation).GetScaledAxis(EAxis::Z);

            // 基于鼠标输入构建移动方向
            FVector ViewMovementDirection = (CameraRight * MouseXInput + CameraUp * MouseYInput).GetSafeNormal();

            // 计算移动强度
            float MovementIntensity = FVector2D(MouseXInput, MouseYInput).Size() * DragSensitivity;

            // 应用移动
            curOldManPullItem->HandleMouseData(ViewMovementDirection, MovementIntensity);

            // 重置鼠标输入
            MouseXInput = 0.0f;
            MouseYInput = 0.0f;
        }
    }
}

void AOldManCharacter::SetCurOldManInterectItem(AOldManInterectItemBase* newItem)
{
    if (newItem)
    {
        curOldManInterectItem = newItem;
    }
}

void AOldManCharacter::ClearCurOldManInterectItem()
{
    curOldManInterectItem = nullptr;
}

void AOldManCharacter::InterectCurOldManInterectItem(FOldManItemInteractData interectData)
{
    if (curOldManInterectItem)
    {
        curOldManInterectItem->Interect(interectData);
    }
}
#pragma endregion