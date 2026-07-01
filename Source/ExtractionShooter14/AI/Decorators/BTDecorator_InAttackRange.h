#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_InAttackRange.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API UBTDecorator_InAttackRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_InAttackRange();

protected:
	virtual bool CalculateRawConditionValue(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory) const override;

	// TargetActor 키 이름 (블랙보드)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName TargetActorKey = "TargetActor";
};
