#pragma once

#include "CoreMinimal.h"
#include "FireMode.generated.h"

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	Single	UMETA(DisplayName = "Single"),
	Burst	UMETA(DisplayName = "Burst"),
	Auto	UMETA(DisplayName = "Auto")
};