#include "BTTask_Attack.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = TEXT("Attack");
	bNotifyTick = true;
}

uint16 UBTTask_Attack::GetInstanceMemorySize() const
{
	return sizeof(FBTAttackMemory);
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	FBTAttackMemory* Memory =
		reinterpret_cast<FBTAttackMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;

	UBlackboardComponent* Blackboard =
		OwnerComp.GetBlackboardComponent();
	if (Blackboard)
	{
		Blackboard->SetValueAsBool(IsAttackKey, true);
	}

	AAIController* AIController =
		OwnerComp.GetAIOwner();
	AZombieCharacter* Zombie =
		AIController
		? Cast<AZombieCharacter>(AIController->GetPawn())
		: nullptr;

	if (!Zombie)
	{
		return EBTNodeResult::Failed;
	}
	if (Zombie->IsDead())
	{
		return EBTNodeResult::Failed;
	}

	// 좀비별 AttackCooldown을 이번 공격 지속시간으로 저장
	Memory->AttackDuration = Zombie->AttackCooldown;

	// 서버에서 모든 클라이언트에게 공격 몽타주 재생
	Zombie->Multicast_PlayAttackMontage();

	AActor* Target =
		Blackboard
		? Cast<AActor>(Blackboard->GetValueAsObject(TEXT("TargetActor")))
		: nullptr;
	if (Target)
	{
		UGameplayStatics::ApplyDamage(
			Target,
			Zombie->AttackDamage,
			AIController,
			Zombie,
			UDamageType::StaticClass());
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	FBTAttackMemory* Memory =
		reinterpret_cast<FBTAttackMemory*>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;

	if (Memory->ElapsedTime >= Memory->AttackDuration)
	{
		if (UBlackboardComponent* Blackboard =
			OwnerComp.GetBlackboardComponent())
		{
			Blackboard->SetValueAsBool(IsAttackKey, false);
		}
		FinishLatentTask(
			OwnerComp,
			EBTNodeResult::Succeeded);
	}
}