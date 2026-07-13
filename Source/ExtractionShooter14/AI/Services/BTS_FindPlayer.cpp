#include "BTS_FindPlayer.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "GameFramework/Character.h"
#include "Character/PlayerCharacter.h"
#include "Character/PlayerCharacterState.h"

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
	AZombieCharacter* Zombie = AIController
		? Cast<AZombieCharacter>(AIController->GetPawn())
		: nullptr;

	if (!Zombie)
	{
		BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
		return;
	}

	const FVector ZombieLocation = Zombie->GetActorLocation();
	AActor* CurrentTarget = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));

	// 현재 타겟이 있으면
	if (CurrentTarget)
	{
		// 타겟이 죽었으면 포기
		APlayerCharacter* CurrentPlayerChar = Cast<APlayerCharacter>(CurrentTarget);
		if (IsValid(CurrentPlayerChar))
		{
			APlayerCharacterState* PS = CurrentPlayerChar->GetPlayerState<APlayerCharacterState>();
			if (IsValid(PS) && PS->GetbIsDead())
			{
				BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
				return;
			}
		}
		else if (!IsValid(CurrentTarget))
		{
			BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
			return;
		}

		// 거리 체크 - GiveUpDistance 초과시 포기
		const float DistanceToTarget = FVector::Dist(
			ZombieLocation,
			CurrentTarget->GetActorLocation());

		if (DistanceToTarget > GiveUpDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Chase Abandoned! Distance: %.1f"),
				*Zombie->ZombieName.ToString(),
				DistanceToTarget);
			BB->SetValueAsObject(TEXT("TargetActor"), nullptr);
			return;
		}
	}

	// 타겟 유무 상관없이 가장 가까운 플레이어 탐지
	// (타겟 있을 때는 더 가까운 플레이어 있으면 전환, 없을 때는 신규 탐지)
	TArray<AActor*> AllPlayers;
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		ACharacter::StaticClass(),
		AllPlayers);

	AActor* NearestPlayer = nullptr;
	float NearestDistance = FLT_MAX;

	for (AActor* Actor : AllPlayers)
	{
		if (Cast<AZombieCharacter>(Actor))
		{
			continue;
		}

		ACharacter* Character = Cast<ACharacter>(Actor);
		if (!Character)
		{
			continue;
		}

		if (!Character->GetController() || !Character->GetController()->IsPlayerController())
		{
			continue;
		}

		// 플레이어가 죽었으면 제외
		APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Character);
		if (IsValid(PlayerChar))
		{
			APlayerCharacterState* PS = PlayerChar->GetPlayerState<APlayerCharacterState>();
			if (IsValid(PS) && PS->GetbIsDead())
			{
				continue;
			}
		}

		const float Distance = FVector::Dist(ZombieLocation, Actor->GetActorLocation());

		// 타겟이 있을 때는 GiveUpDistance 안에서 탐지 (더 가까운 플레이어 전환용)
		// 타겟이 없을 때는 DetectRange 안에서 탐지 (신규 탐지용)
		const float EffectiveRange = CurrentTarget ? GiveUpDistance : Zombie->DetectRange;

		if (Distance <= EffectiveRange && Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestPlayer = Actor;
		}
	}

	BB->SetValueAsObject(TEXT("TargetActor"), NearestPlayer);
}