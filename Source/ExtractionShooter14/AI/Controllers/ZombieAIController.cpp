#include "ZombieAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "../../Characters/Zombie/ZombieCharacter.h"

AZombieAIController::AZombieAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BlackboardComponent =
		CreateDefaultSubobject<UBlackboardComponent>(TEXT("Blackboard"));

	BehaviorTreeComponent =
		CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTree"));
}

void AZombieAIController::BeginPlay()
{
	Super::BeginPlay();

	AZombieCharacter* Zombie = Cast<AZombieCharacter>(GetPawn());

	if (Zombie == nullptr)
	{
		return;
	}

	if (Zombie->BehaviorTree == nullptr)
	{
		return;
	}

	UseBlackboard(
		Zombie->BehaviorTree->BlackboardAsset,
		BlackboardComponent);

	RunBehaviorTree(Zombie->BehaviorTree);
}

void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}