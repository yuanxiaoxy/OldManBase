#include "ApproachEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "OldManEnemyManager.h"

AApproachEnemyCharacter::AApproachEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    _bIsDead = false;
    _DeathTimer = 0.0f;

    // 设置碰撞
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // 创建点击碰撞体
    ClickCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ClickCollision"));
    ClickCollision->SetupAttachment(RootComponent);
    ClickCollision->SetSphereRadius(50.0f);
    ClickCollision->SetCollisionProfileName(FName("UI"));

    // 创建调试球体
    DebugSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugSphere"));
    DebugSphere->SetupAttachment(RootComponent);
    DebugSphere->SetVisibility(false);
}

void AApproachEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 启用点击事件
    EnableInput(GetWorld()->GetFirstPlayerController());
    SetActorEnableCollision(true);
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);

    // 获取玩家控制器
    _PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    _PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    // 绑定点击事件
    OnClicked.AddDynamic(this, &AApproachEnemyCharacter::HandleOnClicked);
    _OriginScale = GetActorScale3D();
    // 设置动态材质
    if (GetMesh())
    {
        UMaterialInterface* BaseMaterial = GetMesh()->GetMaterial(0);
        if (BaseMaterial)
        {
            DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            GetMesh()->SetMaterial(0, DynamicMaterial);
        }
    }
}

void AApproachEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (_bIsDead)
    {
        // 死亡处理
        _DeathTimer -= DeltaTime;
        if (_DeathTimer <= 0.0f)
        {
            Recycle();
        }
        else
        {
            float Alpha = 1.0f - (_DeathTimer / DeathEffectDuration);
            SetActorScale3D((1.0f - Alpha) * _OriginScale);
            if (DynamicMaterial)
            {
                DynamicMaterial->SetScalarParameterValue("Dissolve", Alpha);
            }
        }
    }
    else
    {
        // 核心：更新屏幕空间位置
        UpdateScreenSpacePosition(DeltaTime);
        
        UpdateClickCollision();
        UpdateVisualEffects(DeltaTime);

        // 检查并造成伤害
        CheckAndApplyDamage();
    }
}

// 核心函数：更新屏幕空间位置
void AApproachEnemyCharacter::UpdateScreenSpacePosition(float DeltaTime)
{
    if (!_PlayerController || !_PlayerPawn) return;
    
    // 1. 逐渐减小距离（靠近玩家）
    _CurrentDistance = FMath::FInterpConstantTo(
        _CurrentDistance,
        AttackDistance,  // 目标距离是攻击距离
        DeltaTime,
        ApproachSpeed
    );
    
    // 2. 保持相同的屏幕位置，计算新的世界坐标
    FVector NewWorldLocation = GetWorldPositionFromScreen(_ScreenPosition, _CurrentDistance);
    
    // 3. 应用新位置
    SetActorLocation(NewWorldLocation);
    
    // 4. 使敌人面向摄像机
    FVector CameraLocation = _PlayerPawn->GetActorLocation();
    FVector ToCamera = (CameraLocation - NewWorldLocation).GetSafeNormal();
    FRotator LookAtRotation = ToCamera.Rotation();
    SetActorRotation(LookAtRotation);
}

// 从屏幕坐标获取世界坐标
FVector AApproachEnemyCharacter::GetWorldPositionFromScreen(const FVector2D& ScreenPos, float Distance)
{
    if (!_PlayerController) return FVector::ZeroVector;
    
    // 获取摄像机信息
    FVector CameraLocation;
    FRotator CameraRotation;
    _PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    // 获取视口大小
    int32 ViewportSizeX, ViewportSizeY;
    _PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    
    // 转换为像素坐标
    FVector2D PixelPos(
        ScreenPos.X * ViewportSizeX,
        ScreenPos.Y * ViewportSizeY
    );
    
    // 屏幕坐标转世界坐标
    FVector WorldLocation, WorldDirection;
    if (_PlayerController->DeprojectScreenPositionToWorld(
        PixelPos.X, PixelPos.Y, 
        WorldLocation, WorldDirection))
    {
        return CameraLocation + (WorldDirection * Distance);
    }
    
    // 如果失败，返回摄像机前方
    return CameraLocation + (CameraRotation.Vector() * Distance);
}

// 检查并造成伤害
void AApproachEnemyCharacter::CheckAndApplyDamage()
{
    if (_bIsDead || !_PlayerPawn) return;
    
    float DistanceToPlayer = FVector::Distance(GetActorLocation(), _PlayerPawn->GetActorLocation());
    
    if (DistanceToPlayer <= AttackDistance)
    {
        // 造成伤害
        // UGameplayStatics::ApplyDamage(PlayerPawn, DamagePerSecond, nullptr, this, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Enemy deals damage to player!")); 
        KillEnemy();

    }
}

void AApproachEnemyCharacter::HandleOnClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (_bIsDead || ButtonPressed != EKeys::LeftMouseButton || TouchedActor != this) 
        return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    float MouseX, MouseY;
    if (PC->GetMousePosition(MouseX, MouseY))
    {
        if (IsMouseOverlapping(FVector2D(MouseX, MouseY)))
        {
            KillEnemy();
        }
    }
}

// 初始化敌人（由管理器调用）
void AApproachEnemyCharacter::InitializeEnemy(const FVector2D& InScreenPosition)
{
    _CurrentDistance = InitialDistance;
    _ScreenPosition = InScreenPosition;
    
    
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(true);
    _bIsDead = false;


    // 设置初始位置
    FVector StartLocation = GetWorldPositionFromScreen(_ScreenPosition, _CurrentDistance);
    SetActorLocation(StartLocation);
}



float AApproachEnemyCharacter::GetClickRadius() const
{
    return _CurrentClickRadius;
}

bool AApproachEnemyCharacter::IsMouseOverlapping(const FVector2D& MousePosition) const
{
    if (_bIsDead) return false;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return false;

    int32 ViewportSizeX, ViewportSizeY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    FVector2D EnemyPixelPos = FVector2D(
        _ScreenPosition.X * ViewportSizeX,
        _ScreenPosition.Y * ViewportSizeY
    );

    float Distance = FVector2D::Distance(MousePosition, EnemyPixelPos);
    return Distance <= _CurrentClickRadius;
}

void AApproachEnemyCharacter::KillEnemy()
{
    if (_bIsDead) return;

    _bIsDead = true;
    _DeathTimer = DeathEffectDuration;

    // 禁用碰撞
    ClickCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    

}

void AApproachEnemyCharacter::Recycle()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
    SetActorScale3D(_OriginScale);
    UOldManEnemyManager::GetInstance()->RecycleApproachEnemy(this);
}

void AApproachEnemyCharacter::UpdateClickCollision()
{
    // 计算点击半径：距离越近，点击半径越大
    float DistanceFactor = 1.0f - (_CurrentDistance / InitialDistance);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    _CurrentClickRadius = FMath::Lerp(BaseClickRadius, MaxClickRadius, DistanceFactor);

    // 更新碰撞体大小
    if (ClickCollision)
    {
        ClickCollision->SetSphereRadius(_CurrentClickRadius);
    }
    
    // 更新调试球体大小
    if (DebugSphere)
    {
        DebugSphere->SetWorldScale3D(FVector(_CurrentClickRadius / 50.0f));
    }
}

void AApproachEnemyCharacter::UpdateVisualEffects(float DeltaTime)
{
    if (!DynamicMaterial) return;

    float DistanceFactor = 1.0f - (_CurrentDistance / 1000.0f);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    // 距离越近，效果越强烈
    DynamicMaterial->SetScalarParameterValue("Intensity", 1.0f + DistanceFactor * 2.0f);
    DynamicMaterial->SetScalarParameterValue("PulseSpeed", 1.0f + DistanceFactor * 3.0f);
    
    // 根据距离调整大小
    float Scale = 0.5f + DistanceFactor * 1.5f;  // 0.5倍到2.0倍
    SetActorScale3D(FVector(Scale));
}