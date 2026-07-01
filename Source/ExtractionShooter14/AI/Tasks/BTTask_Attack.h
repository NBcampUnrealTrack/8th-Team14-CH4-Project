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

	// IsAttack 블랙보드 키 (공격 모션/락 표시용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FName IsAttackKey = "IsAttack";

	// 공격 1회 모션이 끝나는 데 걸리는 시간 (애님 길이에 맞춰 조정)
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDuration = 0.5f;
};
