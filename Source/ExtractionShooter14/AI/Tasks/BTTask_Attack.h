#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

struct FBTAttackMemory
{
	float ElapsedTime = 0.0f;
};

UCLASS()
class EXTRACTIONSHOOTER14_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) override;

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	virtual uint16 GetInstanceMemorySize() const override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName IsAttackKey = "IsAttack";

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDuration = 0.5f;
};
