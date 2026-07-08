#pragma once

#include "CoreMinimal.h"
#include "CollectCombatInfo.generated.h"

USTRUCT(BlueprintType)
struct FCollectCombatInfo
{
	GENERATED_BODY()
//===================== Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DamageDealt = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float DamageTaken = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 KillCount = 0;
//====================== Root
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float SurviveTime = 0.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LootCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Score = 0;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Rank = 1;
};
