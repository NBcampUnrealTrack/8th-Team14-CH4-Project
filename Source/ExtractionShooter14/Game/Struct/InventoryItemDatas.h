#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDatas.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItemData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName = FName(TEXT(""));
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemCount = 0;
};