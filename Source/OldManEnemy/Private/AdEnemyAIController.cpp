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