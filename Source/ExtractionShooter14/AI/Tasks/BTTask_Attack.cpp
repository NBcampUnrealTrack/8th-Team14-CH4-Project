#include "BTTask_Attack.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

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
	FBTAttackMemory* MyMemory = reinterpret_cast<FBTAttackMemory*>(NodeMemory);
	MyMemory->ElapsedTime = 0.0f;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Yellow,
			TEXT("BTTask_Attack: ExecuteTask Called"));
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsBool(IsAttackKey, true);
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	AZombieCharacter* Zombie = AIController ? Cast<AZombieCharacter>(AIController->GetPawn()) : nullptr;

	if (!Zombie || Zombie->bIsDead)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 2.f, FColor::Orange,
				TEXT("BTTask_Attack: Zombie is NULL or Dead"));
		}
		return EBTNodeResult::Failed;
	}

	if (Zombie->AttackMontage)
	{
		Zombie->PlayAnimMontage(Zombie->AttackMontage);
	}

	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor"))) : nullptr;
	if (Target)
	{
		UGameplayStatics::ApplyDamage(
			Target,
			Zombie->AttackDamage,
			AIController,
			Zombie,
			UDamageType::StaticClass());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 2.f, FColor::Red,
				FString::Printf(TEXT("[%s] Attack! Damage: %.1f"),
					*Zombie->ZombieName.ToString(),
					Zombie->AttackDamage));
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1, 2.f, FColor::Orange,
				TEXT("BTTask_Attack: Target is NULL"));
		}
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_Attack::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	FBTAttackMemory* MyMemory = reinterpret_cast<FBTAttackMemory*>(NodeMemory);
	MyMemory->ElapsedTime += DeltaSeconds;

	if (MyMemory->ElapsedTime >= AttackDuration)
	{
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool(IsAttackKey, false);
		}

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}