#pragma once

#include "CoreMinimal.h"
#include "PlayerDatas.generated.h"

USTRUCT(BlueprintType)
struct FPlayerDatas
{
	GENERATED_BODY()
public:
	UPROPERTY()
	int32 Gold;
	UPROPERTY()
	TArray<FName> ItemIDs;
};