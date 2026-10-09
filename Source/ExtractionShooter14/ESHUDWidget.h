#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ESHUDWidget.generated.h"

class UEscapeProgressWidget;
class UCanvasPanel;
class APlayerCharacterState;
class ATestGameState;
class UTextBlock;

UCLASS()
class EXTRACTIONSHOOTER14_API UESHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void ApplyTextColorAnimationByTime(float InRemainTime);
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void BP_UpdatePlayerHP(float CurrentHP, float MaxHP);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "HUD")
	void BP_PlayerDamageFlash();
	
	void CreateProgressWidget(UEscapeProgressWidget* InEPWidget);
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> StageCanvasPanel;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CurrentRank;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StageTime;
	
	FTimerHandle StageTimerHandle;
	void HandleStageTime();
	UFUNCTION()
	void UpdateStageTimerText();
	
	FDelegateHandle PlayerRankDelegateHandle;
	void HandlePlayerRank();
	
	UPROPERTY()
	TObjectPtr<ATestGameState> OwningGameState;
	UPROPERTY()
	TObjectPtr<APlayerCharacterState> OwningPlayerState;

	void SetOwningGameState(ATestGameState* InGameState);
	void SetOwningPlayerState(APlayerCharacterState* InPlayerCharacterState);
};