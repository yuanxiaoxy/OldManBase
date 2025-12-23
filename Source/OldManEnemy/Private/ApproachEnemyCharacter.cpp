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

    m_bIsDead = false;
    m_deathTimer = 0.0f;

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
    m_playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    m_playerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    // 绑定点击事件
   /* OnClicked.AddDynamic(this, &AApproachEnemyCharacter::HandleOnClicked);*/
    m_originScale = GetActorScale3D();
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

    if (m_bIsDead)
    {
        // 死亡处理
        m_deathTimer -= DeltaTime;
        if (m_deathTimer <= 0.0f)
        {
            Recycle();
        }
        else
        {
            float Alpha = 1.0f - (m_deathTimer / DeathEffectDuration);
            SetActorScale3D((1.0f - Alpha) * m_originScale);
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
        
        //UpdateClickCollision();
        UpdateVisualEffects(DeltaTime);

        // 检查并造成伤害
        CheckAndApplyDamage();
    }
}

// 核心函数：更新屏幕空间位置
void AApproachEnemyCharacter::UpdateScreenSpacePosition(float DeltaTime)
{
    if (!m_playerController || !m_playerPawn) return;
    
    // 1. 逐渐减小距离（靠近玩家）
    m_currentDistance = FMath::FInterpConstantTo(
        m_currentDistance,
        m_attackDistance,  // 目标距离是攻击距离
        DeltaTime,
        m_approachSpeed
    );
    
    // 2. 保持相同的屏幕位置，计算新的世界坐标
    FVector NewWorldLocation = GetWorldPositionFromScreen(m_screenPosition, m_currentDistance);
    
    // 3. 应用新位置
    SetActorLocation(NewWorldLocation);
    
    // 4. 使敌人面向摄像机
    FVector CameraLocation = m_playerPawn->GetActorLocation();
    FVector ToCamera = (CameraLocation - NewWorldLocation).GetSafeNormal();
    FRotator LookAtRotation = ToCamera.Rotation();
    SetActorRotation(LookAtRotation);
}

// 从屏幕坐标获取世界坐标
FVector AApproachEnemyCharacter::GetWorldPositionFromScreen(const FVector2D& ScreenPos, float Distance)
{
    if (!m_playerController) return FVector::ZeroVector;
    
    // 获取摄像机信息
    FVector CameraLocation;
    FRotator CameraRotation;
    m_playerController->GetPlayerViewPoint(CameraLocation, CameraRotation);
    
    // 获取视口大小
    int32 ViewportSizeX, ViewportSizeY;
    m_playerController->GetViewportSize(ViewportSizeX, ViewportSizeY);
    
    // 转换为像素坐标
    FVector2D PixelPos(
        ScreenPos.X * ViewportSizeX,
        ScreenPos.Y * ViewportSizeY
    );
    
    // 屏幕坐标转世界坐标
    FVector WorldLocation, WorldDirection;
    if (m_playerController->DeprojectScreenPositionToWorld(
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
    if (m_bIsDead || !m_playerPawn) return;
    
    float DistanceToPlayer = FVector::Distance(GetActorLocation(), m_playerPawn->GetActorLocation());
    
    if (DistanceToPlayer <= m_attackDistance)
    {
        // 造成伤害
        // UGameplayStatics::ApplyDamage(PlayerPawn, DamagePerSecond, nullptr, this, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Enemy deals damage to player!")); 
        UOldManEnemyManager::GetInstance()->ShootInk(m_screenPosition, m_playerController);
        KillEnemy();

    }
}

//void AApproachEnemyCharacter::HandleOnClicked(AActor* TouchedActor, FKey ButtonPressed)
//{
//    if (m_bIsDead || ButtonPressed != EKeys::LeftMouseButton || TouchedActor != this) 
//        return;
//
//    APlayerController* PC = GetWorld()->GetFirstPlayerController();
//    if (!PC) return;
//
//    float MouseX, MouseY;
//    if (PC->GetMousePosition(MouseX, MouseY))
//    {
//        if (IsMouseOverlapping(FVector2D(MouseX, MouseY)))
//        {
//            KillEnemy();
//        }
//    }
//}

// 初始化敌人（由管理器调用）
void AApproachEnemyCharacter::InitializeEnemy(
    const FVector2D& InScreenPosition,
    float attackDistance,
    float approachSpeed,
    float initialDistacne)
{
    m_initialDistance = initialDistacne;
    m_currentDistance = m_initialDistance;
    m_screenPosition = InScreenPosition;
    m_attackDistance = attackDistance;
    m_approachSpeed = approachSpeed;
    
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    SetActorTickEnabled(true);
    m_bIsDead = false;


    // 设置初始位置
    FVector StartLocation = GetWorldPositionFromScreen(m_screenPosition, m_currentDistance);
    SetActorLocation(StartLocation);
}



//float AApproachEnemyCharacter::GetClickRadius() const
//{
//    return m_currentClickRadius;
//}
//
//bool AApproachEnemyCharacter::IsMouseOverlapping(const FVector2D& MousePosition) const
//{
//    if (m_bIsDead) return false;
//
//    APlayerController* PC = GetWorld()->GetFirstPlayerController();
//    if (!PC) return false;
//
//    int32 ViewportSizeX, ViewportSizeY;
//    PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
//
//    FVector2D EnemyPixelPos = FVector2D(
//        m_screenPosition.X * ViewportSizeX,
//        m_screenPosition.Y * ViewportSizeY
//    );
//
//    float Distance = FVector2D::Distance(MousePosition, EnemyPixelPos);
//    return Distance <= m_currentClickRadius;
//}

void AApproachEnemyCharacter::KillEnemy()
{
    if (m_bIsDead) return;

    m_bIsDead = true;
    m_deathTimer = DeathEffectDuration;

    // 禁用碰撞
    ClickCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    

}

void AApproachEnemyCharacter::Recycle()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
    SetActorScale3D(m_originScale);
    UOldManEnemyManager::GetInstance()->RecycleApproachEnemy(this);
}

//void AApproachEnemyCharacter::UpdateClickCollision()
//{
//    // 计算点击半径：距离越近，点击半径越大
//    float DistanceFactor = 1.0f - (_CurrentDistance / InitialDistance);
//    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);
//
//    _CurrentClickRadius = FMath::Lerp(BaseClickRadius, MaxClickRadius, DistanceFactor);
//
//    // 更新碰撞体大小
//    if (ClickCollision)
//    {
//        ClickCollision->SetSphereRadius(_CurrentClickRadius);
//    }
//    
//    // 更新调试球体大小
//    if (DebugSphere)
//    {
//        DebugSphere->SetWorldScale3D(FVector(_CurrentClickRadius / 50.0f));
//    }
//}

void AApproachEnemyCharacter::UpdateVisualEffects(float DeltaTime)
{
    if (!DynamicMaterial) return;

    float DistanceFactor = 1.0f - (m_currentDistance / 1000.0f);
    DistanceFactor = FMath::Clamp(DistanceFactor, 0.0f, 1.0f);

    // 距离越近，效果越强烈
    DynamicMaterial->SetScalarParameterValue("Intensity", 1.0f + DistanceFactor * 2.0f);
    DynamicMaterial->SetScalarParameterValue("PulseSpeed", 1.0f + DistanceFactor * 3.0f);
    
    // 根据距离调整大小
    float Scale = 0.5f + DistanceFactor * 1.5f;  // 0.5倍到2.0倍
    SetActorScale3D(FVector(Scale));
}