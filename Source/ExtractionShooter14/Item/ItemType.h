#pragma once

#include "CoreMinimal.h"
#include "ItemType.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	Coin		UMETA(DisplayName = "Coin"),
	Consumable UMETA(DisplayName = "Consumable"),
	Quest		UMETA(DisplayName = "Quest"),
	Etc			UMETA(DisplayName = "Etc")
};