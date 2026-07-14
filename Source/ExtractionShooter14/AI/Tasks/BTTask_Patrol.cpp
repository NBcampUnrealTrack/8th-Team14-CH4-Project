#include "BTTask_Patrol.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

UBTTask_Patrol::UBTTask_Patrol()
{
	NodeName = TEXT("Patrol");
	bNotifyTick = true;
}

uint16 UBTTask_Patrol::GetInstanceMemorySize() const
{
	return sizeof(FBTPatrolMemory);
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("Execute Patrol"));

	FBTPatrolMemory* Memory = reinterpret_cast<FBTPatrolMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;
	Memory->bIsWaiting = false;
	Memory->bIsReturningHome = false;

	AAIController* AIController = OwnerComp.GetAIOwner();
	AZombieCharacter* Zombie = AIController
		? Cast<AZombieCharacter>(AIController->GetPawn())
		: nullptr;

	if (!Zombie)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}

	const FVector HomeLocation = BB->GetValueAsVector(TEXT("HomeLocation"));
	const FVector ZombieLocation = Zombie->GetActorLocation();
	const float DistFromHome = FVector::Dist(ZombieLocation, HomeLocation);

	// 집에서 멀면 집으로 귀환
	if (DistFromHome > Zombie->PatrolRadius)
	{
		Memory->bIsReturningHome = true;
		Memory->PatrolTarget = HomeLocation;

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(HomeLocation);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		AIController->MoveTo(MoveRequest);

		UE_LOG(LogTemp, Warning, TEXT("[Patrol] Returning Home..."));
	}
	else
	{
		// 집 근처면 랜덤 위치로 배회
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

		FNavLocation RandomLocation;
		bool bFound = false;

		if (NavSys)
		{
			for (int32 i = 0; i < 5; i++)
			{
				if (NavSys->GetRandomReachablePointInRadius(
					HomeLocation,
					Zombie->PatrolRadius,
					RandomLocation))
				{
					bFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			RandomLocation.Location = HomeLocation;
		}

		Memory->PatrolTarget = RandomLocation.Location;
		Memory->WaitTime = FMath::FRandRange(
			Zombie->PatrolWaitTimeMin,
			Zombie->PatrolWaitTimeMax);

		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(Memory->PatrolTarget);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		AIController->MoveTo(MoveRequest);

		UE_LOG(LogTemp, Warning, TEXT("[Patrol] Patrolling to: %s"),
			*Memory->PatrolTarget.ToString());
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Patrol::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{

	FBTPatrolMemory* Memory = reinterpret_cast<FBTPatrolMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	AZombieCharacter* Zombie = AIController
		? Cast<AZombieCharacter>(AIController->GetPawn())
		: nullptr;

	if (!Zombie)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 대기 중
	if (Memory->bIsWaiting)
	{
		Memory->ElapsedTime += DeltaSeconds;
		if (Memory->ElapsedTime >= Memory->WaitTime)
		{
			UE_LOG(LogTemp, Warning, TEXT("PATROL FINISHED (Wait End)"));
			// 대기 끝 - 다시 Patrol 시작
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		return;
	}

	//// 목적지 도착 체크
	//const float DistToTarget = FVector::Dist(
	//	Zombie->GetActorLocation(),
	//	Memory->PatrolTarget);

	//if (DistToTarget <= AcceptanceRadius)
	//{
	//	if (Memory->bIsReturningHome)
	//	{
	//		UE_LOG(LogTemp, Warning, TEXT("PATROL FINISHED (Return Home)"));

	//		// 집 도착 - 배회 시작
	//		Memory->bIsReturningHome = false;
	//		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	//	}
	//	else
	//	{
	//		// 배회 목적지 도착 - 대기 시작
	//		Memory->bIsWaiting = true;
	//		Memory->ElapsedTime = 0.0f;
	//		AIController->StopMovement();
	//	}
	//}
	// AI가 실제 이동을 끝냈는지 확인

	UE_LOG(LogTemp, Warning,
		TEXT("MoveStatus = %d"),
		(int32)AIController->GetMoveStatus());

	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		if (Memory->bIsReturningHome)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Patrol] Return Home Complete"));

			Memory->bIsReturningHome = false;
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Patrol] Arrived Patrol Point"));

			Memory->bIsWaiting = true;
			Memory->ElapsedTime = 0.0f;
		}
	}
}