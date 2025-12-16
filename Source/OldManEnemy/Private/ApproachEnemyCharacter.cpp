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

    bIsDead = false;
    DeathTimer = 0.0f;
    CurrentDistance = 1000.0f;  // 初始距离
    ApproachSpeed = 100.0f;     // 靠近速度（单位/秒）
    AttackDistance = 200.0f;    // 攻击距离

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
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    // 绑定点击事件
    OnClicked.AddDynamic(this, &AApproachEnemyCharacter::HandleOnClicked);

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

    if (bIsDead)
    {
        // 死亡处理
        DeathTimer -= DeltaTime;
        if (DeathTimer <= 0.0f)
        {
            Recycle();
        }
        else
        {
            float Alpha = 1.0f - (DeathTimer / DeathEffectDuration);
            SetActorScale3D(FVector(1.0f - Alpha));
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
    if (!PlayerController || !PlayerPawn) return;
    
    // 1. 逐渐减小距离（靠近玩家）
    CurrentDistance = FMath::FInterpConstantTo(
        CurrentDistance,
        AttackDistance,  // 目标距离是攻击距离
        DeltaTime,
        ApproachSpeed
    );
    
    // 2. 保持相同的屏幕位置，计算新的世界坐标
    FVector NewWorldLocation = GetWorldPositionFromScreen(ScreenPosition, CurrentDistance);
    
    // 3. 应用新位置
    SetActorLocation(NewWorldLocation);
    
    // 4. 使敌人面向摄像机
    FVector CameraLocation = PlayerPawn->GetActorLocation();
    FVector ToCamera = (CameraLocation - NewWorldLocation).GetSafeNormal();
    FRotator LookAtRotation = ToCamera.Rotation();
    SetActorRotation(LookAtRotation);
}

// 从屏幕坐标获取世界坐标
FVector AApproachEnemyCharacter::GetWorldPositionFromScreen(const FVector2D& ScreenPos, float Distance)
{
    if (!PlayerController) return FVector::ZeroVector;
    
    // 获取摄像机信息
    FVector CameraLocation;
    FRotator CameraRotation;
    PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    // 获取视口大小
    int32 ViewportSizeX, ViewportSizeY;
    PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    
    // 转换为像素坐标
    FVector2D PixelPos(
        ScreenPos.X * ViewportSizeX,
        ScreenPos.Y * ViewportSizeY
    );
    
    // 屏幕坐标转世界坐标
    FVector WorldLocation, WorldDirection;
    if (PlayerController->DeprojectScreenPositionToWorld(
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
    if (bIsDead || !PlayerPawn) return;
    
    float DistanceToPlayer = FVector::Distance(GetActorLocation(), PlayerPawn->GetActorLocation());
    
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
    if (bIsDead || ButtonPressed != EKeys::LeftMouseButton || TouchedActor != this) 
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
void AApproachEnemyCharacter::InitializeEnemy(const FVector2D& InScreenPosition, float InitialDistance, 
                                             float InAttackDistance, float InApproachSpeed)
{
    MaxDistance = InitialDistance;
    ScreenPosition = InScreenPosition;
    CurrentDistance = InitialDistance;
    AttackDistance = InAttackDistance;
    ApproachSpeed = InApproachSpeed;
    
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(true);
    bIsDead = false;


    // 设置初始位置
    FVector StartLocation = GetWorldPositionFromScreen(ScreenPosition, CurrentDistance);
    SetActorLocation(StartLocation);
}

void AApproachEnemyCharacter::SetScreenPosition(const FVector2D& InScreenPos, float InCurrentDistance)
{
    ScreenPosition = InScreenPos;
    CurrentDistance = InCurrentDistance;
    UpdateClickCollision();
}

float AApproachEnemyCharacter::GetClickRadius() const
{
    return CurrentClickRadius;
}

bool AApproachEnemyCharacter::IsMouseOverlapping(const FVector2D& MousePosition) const
{
    if (bIsDead) return false;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return false;

    int32 ViewportSizeX, ViewportSizeY;
    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

    FVector2D EnemyPixelPos = FVector2D(
        ScreenPosition.X * ViewportSizeX,
        ScreenPosition.Y * ViewportSizeY
    );

    float Distance = FVector2D::Distance(MousePosition, EnemyPixelPos);
    return Distance <= CurrentClickRadius;
}

void AApproachEnemyCharacter::KillEnemy()
{
    if (bIsDead) return;

    bIsDead = true;
    DeathTimer = DeathEffectDuration;

    // 禁用碰撞
    ClickCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    

}

void AApproachEnemyCharacter::Recycle()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
    UOldManEnemyManager::GetInstance()->RecycleApproachEnemy(this);
}

void AApproachEnemyCharacter::UpdateClickCollision()
{
    // 计算点击半径：距离越近，点击半径越大
    float DistanceFactor = 1.0f - (CurrentDistance / MaxDistance);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    CurrentClickRadius = FMath::Lerp(BaseClickRadius, MaxClickRadius, DistanceFactor);

    // 更新碰撞体大小
    if (ClickCollision)
    {
        ClickCollision->SetSphereRadius(CurrentClickRadius);
    }
    
    // 更新调试球体大小
    if (DebugSphere)
    {
        DebugSphere->SetWorldScale3D(FVector(CurrentClickRadius / 50.0f));
    }
}

void AApproachEnemyCharacter::UpdateVisualEffects(float DeltaTime)
{
    if (!DynamicMaterial) return;

    float DistanceFactor = 1.0f - (CurrentDistance / 1000.0f);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    // 距离越近，效果越强烈
    DynamicMaterial->SetScalarParameterValue("Intensity", 1.0f + DistanceFactor * 2.0f);
    DynamicMaterial->SetScalarParameterValue("PulseSpeed", 1.0f + DistanceFactor * 3.0f);
    
    // 根据距离调整大小
    float Scale = 0.5f + DistanceFactor * 1.5f;  // 0.5倍到2.0倍
    SetActorScale3D(FVector(Scale));
}