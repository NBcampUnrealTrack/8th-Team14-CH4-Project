#pragma once

#include "CoreMinimal.h"
#include "PlayerDatas.generated.h"

USTRUCT(BlueprintType)
struct FPlayerDatas
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PlayerNickName = TEXT("Player");
	UPROPERTY()
	int32 Gold = 0;
	UPROPERTY()
	TArray<FName> ItemNames;
};