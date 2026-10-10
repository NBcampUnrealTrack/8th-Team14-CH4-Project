#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UseItemData.generated.h"

class UStaticMesh;

// DT_UseItem row : how a usable item (bandage, medkit ...) works
// sale / display info lives in DT_ShopItem (ItemType UseItem, linked by FItemData::DataRowName)
USTRUCT(BlueprintType)
struct FUseItemData : public FTableRowBase
{
	GENERATED_BODY()

public:

	// shop price (server reads this, client never sends it)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shop")
	int32 Price = 0;

	// seconds needed to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UseTime = 0.f;

	// effect value on use (heal amount for heal items)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectAmount = 0.f;

	// look when lying in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMesh> Mesh = nullptr;
};
