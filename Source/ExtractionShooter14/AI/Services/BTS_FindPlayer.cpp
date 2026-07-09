#include "BTS_FindPlayer.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "GameFramework/Character.h"

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
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		return;
	}

	// 모든 플레이어 중 가장 가까운 플레이어 찾기
	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		ACharacter::StaticClass(),
		AllPlayers);

	AActor* NearestPlayer = nullptr;
	float NearestDistance = FLT_MAX;

	for (AActor* Actor : AllPlayers)
	{
		// 좀비는 제외
		if (Cast<AZombieCharacter>(Actor))
		{
			continue;
		}

		// AI 컨트롤러가 있는 폰은 제외 (좀비 등)
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character)
		{
			continue;
		}

		// 플레이어 컨트롤러가 있는 것만
		if (!Character->GetController() || !Character->GetController()->IsPlayerController())
		{
			continue;
		}

		const float Distance = FVector::Dist(
			Zombie->GetActorLocation(),
			Actor->GetActorLocation());

		if (Distance <= Zombie->DetectRange && Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPlayer = Actor;
		}
	}

	BB->SetValueAsObject(TEXT("TargetActor"), NearestPlayer);
}