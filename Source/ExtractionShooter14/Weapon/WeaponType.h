#pragma once

#include "CoreMinimal.h"
#include "WeaponType.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	Sniper		UMETA(DisplayName = "Sniper"),
	Grenade		UMETA(DisplayName = "Grenade")
};