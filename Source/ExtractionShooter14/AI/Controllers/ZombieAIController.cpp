#include "ZombieAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"

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

	GEngine->AddOnScreenDebugMessage(
		-1, 5.f, FColor::Red,
		TEXT("ZombieAIController BeginPlay"));

	if (BehaviorTree)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.f, FColor::Green,
			TEXT("Run Behavior Tree"));
		RunBehaviorTree(BehaviorTree);
	}
}

void AZombieAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// RunBehaviorTree 이후 블랙보드가 초기화된 다음에 HomeLocation 설정
	if (BlackboardComponent && InPawn)
	{
		BlackboardComponent->SetValueAsVector(
			TEXT("HomeLocation"),
			InPawn->GetActorLocation());

		UE_LOG(LogTemp, Warning, TEXT("[ZombieAIController] HomeLocation Set: %s"),
			*InPawn->GetActorLocation().ToString());
	}
}

void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}