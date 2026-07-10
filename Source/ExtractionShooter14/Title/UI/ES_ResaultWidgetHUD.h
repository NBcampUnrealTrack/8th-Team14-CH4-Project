#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/Struct/CollectCombatInfo.h"
#include "ES_ResaultWidgetHUD.generated.h"

class UTextBlock;
class UES_EndWidgetSection;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_ResaultWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void SetPlayerCombatInfo(const FCollectCombatInfo& CombatInfo);
	void SetPlayerNickName(const FText& NickName);
	UFUNCTION()
	void OnFadeInAnimation_Com_Finished();
	void AddRowInComBatSection();
	
	UFUNCTION()
	void OnFadeInAnimation_Col_Finished();
	void AddRowInCollectSection();
	
	UFUNCTION()
	void AddCombatRowByTime();
	UFUNCTION()
	void AddCollectRowByTime();
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UES_EndWidgetSection> CombatSectionWidget;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UES_EndWidgetSection> CollectSectionWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NickNameTextBlock;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RankTextBlock;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReturnToLobbyTimeText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReturnToLobbyText;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_Com;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_Col;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_ReL;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation_ReT;
	
	UPROPERTY()
	FCollectCombatInfo PlayerCombatInfo;
	
	FDelegateHandle PlayerCommitInfoDelegateHandle;
	
	FTimerHandle ReturnToLobbyTimerHandle;
	int32 ReturnToLobbyTime = 4;
	
	FTimerHandle CombatRowTimerHandle;
	FTimerHandle CollectRowTimerHandle;

	int32 CombatInfoArrayIndex = 0;
	int32 CollectInfoArrayIndex = 0;

	TArray<TPair<FText, FText>> CombatInfoArray;
	TArray<TPair<FText, FText>> CollectInfoArray;
};
