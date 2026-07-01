#include "BTS_FindPlayer.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Characters/Zombie/ZombieCharacter.h"

UBTS_FindPlayer::UBTS_FindPlayer()
{
	NodeName = TEXT("Find Player");
	bNotifyTick = true;
}

void UBTS_FindPlayer::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	AZombieCharacter* Zombie = AIController ? Cast<AZombieCharacter>(AIController->GetPawn()) : nullptr;
	if (!Zombie)
	{
		return;
	}

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		return;
	}

	const float Distance = FVector::Dist(Zombie->GetActorLocation(), Player->GetActorLocation());

	if (Distance <= Zombie->DetectRange)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), Player);
	}
	else
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
	}
}