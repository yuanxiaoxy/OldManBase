// AdEnemyAIController.cpp
#include "AdEnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AdEnemyCharacter.h"


AAdEnemyAIController::AAdEnemyAIController()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AAdEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	EnemyCharacter = Cast<AAdEnemyCharacter>(GetPawn());
}

void AAdEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AAdEnemyCharacter* EnemyCharacter = Cast<AAdEnemyCharacter>(InPawn);
	if (EnemyCharacter && EnemyCharacter->BehaviorTree && EnemyCharacter->BehaviorTree->BlackboardAsset)
	{
		BlackboardComponent->InitializeBlackboard(*(EnemyCharacter->BehaviorTree->BlackboardAsset));
		BehaviorTreeComponent->StartTree(*EnemyCharacter->BehaviorTree);
	}
}

void AAdEnemyAIController::SetTargetPlayer(AActor* PlayerActor)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject("TargetPlayer", PlayerActor);
	}
}

void AAdEnemyAIController::NotifyOtherMonsters(AActor* SpottedPlayer)
{
	// 实现通知逻辑
}

void AAdEnemyAIController::ChangeState(EAdMonsterState state) 
{
	// 确保指针有效
	if (!EnemyCharacter || !BlackboardComponent)
	{
		return;
	}

	// 更新角色状态
	EnemyCharacter->ChangeState(state);

	// 更新行为树黑板（显式转换为 uint8）
	BlackboardComponent->SetValueAsEnum("CurrentState", static_cast<uint8>(state));

	// 可选：添加调试输出
	UE_LOG(LogTemp, Warning, TEXT("Enemy state changed to: %d"), static_cast<uint8>(state));
}


bool AAdEnemyAIController::IsPlayerDetected(float radius)
{
	// 1. 先检查所有必要的指针是否有效
	if (!BlackboardComponent || !GetPawn())
	{
		return false;
	}

	AActor* PlayerActor = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetPlayer"));

	// 2. 添加更严格的空指针检查
	if (!PlayerActor || !EnemyCharacter)
	{
		return false;
	}

	// 3. 计算XY平面距离
	float DistanceToPlayer = FVector::DistXY(EnemyCharacter->GetActorLocation(),
		PlayerActor->GetActorLocation());

	// 4. 距离检测
	if (DistanceToPlayer <= radius)
	{
		// 5. 改变状态并返回true
		ChangeState(EAdMonsterState::Tracking);
		return true;
	}

	return false;
}