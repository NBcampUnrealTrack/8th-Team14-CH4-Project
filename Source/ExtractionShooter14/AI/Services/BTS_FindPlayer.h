#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BTS_FindPlayer.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API UBTS_FindPlayer : public UBTService
{
	GENERATED_BODY()
public:
	UBTS_FindPlayer();

protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds) override;

	// 좀비와 플레이어 사이 거리가 이 이상이면 추격 포기
	UPROPERTY(EditAnywhere, Category = "AI")
	float GiveUpDistance = 1500.0f;
};