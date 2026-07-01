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
        -1,
        5.f,
        FColor::Red,
        TEXT("ZombieAIController BeginPlay"));
    if (BehaviorTree)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            5.f,
            FColor::Green,
            TEXT("Run Behavior Tree"));
        RunBehaviorTree(BehaviorTree);

        // 빙의한 좀비의 스폰 위치를 HomeLocation 키에 저장
        if (BlackboardComponent && GetPawn())
        {
            BlackboardComponent->SetValueAsVector(
                TEXT("HomeLocation"),
                GetPawn()->GetActorLocation());
        }
    }
}

void AZombieAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}