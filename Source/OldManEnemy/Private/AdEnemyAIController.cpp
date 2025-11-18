// AdEnemyAIController.cpp
#include "AdEnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AdEnemyCharacter.h"
#include "OldManEnemyManager.h"



AAdEnemyAIController::AAdEnemyAIController()
{
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AAdEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
	EnemyCharacter = Cast<AAdEnemyCharacter>(GetPawn());
	TSingleton<UOldManEnemyManager>::GetInstance()->Enemys.Add(this);
}

void AAdEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("AAdEnemyAIController.OnProssess is called."));
	AAdEnemyCharacter* PossessedEnemy = Cast<AAdEnemyCharacter>(InPawn);
	if (!PossessedEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to possess pawn as AAdEnemyCharacter!"));
		return;
	}


	if (PossessedEnemy->BehaviorTree && PossessedEnemy->BehaviorTree->BlackboardAsset)
	{
		// 初始化黑板组件
		if (BlackboardComponent && BlackboardComponent->InitializeBlackboard(*(PossessedEnemy->BehaviorTree->BlackboardAsset)))
		{
			// 启动行为树
			RunBehaviorTree(PossessedEnemy->BehaviorTree);
			UE_LOG(LogTemp, Warning, TEXT("Behavior Tree started successfully for %s!"), *PossessedEnemy->GetName());
		}
	}
	
}
void AAdEnemyAIController::Tick(float DeltaTime)
{
	
}

EAdMonsterState AAdEnemyAIController::GetCurrentState()
{
	return EnemyCharacter->CurrentState;
}

void AAdEnemyAIController::NotifyOtherMonsters()
{
	if (hasTracked)
		return;
	TSingleton<UOldManEnemyManager>::GetInstance()->NotifyMonstersTracking();
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
	//UE_LOG(LogTemp, Warning, TEXT("Enemy state changed to: %d"), static_cast<uint8>(state));
}


