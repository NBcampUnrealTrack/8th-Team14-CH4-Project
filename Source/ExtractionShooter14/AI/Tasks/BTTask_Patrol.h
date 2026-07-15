#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Patrol.generated.h"

struct FBTPatrolMemory
{
	FVector PatrolTarget = FVector::ZeroVector;
	float WaitTime = 0.0f;
	float ElapsedTime = 0.0f;
	bool bIsWaiting = false;
	bool bIsReturningHome = false;
};

UCLASS()
class EXTRACTIONSHOOTER14_API UBTTask_Patrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Patrol();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual uint16 GetInstanceMemorySize() const override;

	UPROPERTY(EditAnywhere, Category = "Patrol")
	float AcceptanceRadius = 100.0f;
private:
	void MoveToLocation(
		AAIController* AIController,
		const FVector& TargetLocation);
};
