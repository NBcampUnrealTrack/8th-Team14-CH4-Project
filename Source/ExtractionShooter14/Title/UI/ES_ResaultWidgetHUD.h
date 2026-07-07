#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/Struct/CollectCombatInfo.h"
#include "ES_ResaultWidgetHUD.generated.h"

class UES_EndWidgetSection;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_ResaultWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void SetPlayerCombatInfo(const FCollectCombatInfo& CombatInfo);
	
	UFUNCTION()
	void OnFadeInAnimation_Com_Finished();
	void AddRowInComBatSection();
	
	UFUNCTION()
	void OnFadeInAnimation_Col_Finished();
	void AddRowInCollectSection();
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UES_EndWidgetSection> CombatSectionWidget;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UES_EndWidgetSection> CollectSectionWidget;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_Com;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_Col;
	
	UPROPERTY()
	FCollectCombatInfo PlayerCombatInfo;
};
