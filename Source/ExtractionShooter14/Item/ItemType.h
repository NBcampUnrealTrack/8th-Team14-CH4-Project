#pragma once

#include "CoreMinimal.h"
#include "ItemType.generated.h"

// picks the performance table a DT_ShopItem row points into
//   Weapon  -> DT_Weapon
//   UseItem -> DT_UseItem
UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	UseItem		UMETA(DisplayName = "UseItem")
};
