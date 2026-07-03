#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ESHUDWidget.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API UESHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void BP_UpdatePlayerHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void BP_PlayerDamageFlash();
};