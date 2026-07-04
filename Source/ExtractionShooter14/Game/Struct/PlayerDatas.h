#pragma once

#include "CoreMinimal.h"
#include "Game/Struct/InventoryItemDatas.h"
#include "PlayerDatas.generated.h"

USTRUCT(BlueprintType)
struct FPlayerDatas
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PlayerNickName = TEXT("Player");
	UPROPERTY()
	int32 GoldStorage = 0;
	UPROPERTY()
	TArray<FInventoryItemData> ItemStorage;
};
//			================================
//			  PREVIEW FINVENTORYITEMDATA.H
//			================================
//				USTRUCT(BlueprintType)
//				struct FInventoryItemData
//				{
//					GENERATED_BODY()
//					FName ItemName = FName(TEXT(""));
//					int32 ItemCount = 0;
//				 };