#include "BTDecorator_InAttackRange.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Engine/Engine.h"

UBTDecorator_InAttackRange::UBTDecorator_InAttackRange()
{
	NodeName = TEXT("In Attack Range");
}

bool UBTDecorator_InAttackRange::CalculateRawConditionValue(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return false;
	}

	AActor* TargetActor = Cast<AActor>(BB->GetValueAsObject(TargetActorKey));
	if (!TargetActor)
	{
		return false;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	AZombieCharacter* Zombie = Cast<AZombieCharacter>(AIController->GetPawn());
	if (!Zombie)
	{
		return false;
	}

	const float Distance = FVector::Dist(
		Zombie->GetActorLocation(),
		TargetActor->GetActorLocation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 0.1f, FColor::Cyan,
			FString::Printf(TEXT("Distance: %.1f / AttackRange: %.1f"),
				Distance, Zombie->AttackRange));
	}

	return Distance <= Zombie->AttackRange;
}