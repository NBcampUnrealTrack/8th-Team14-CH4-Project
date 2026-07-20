#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EscapeProgressWidget.generated.h"

class ATestGameState;
class UProgressBar;
class UTextBlock;

UCLASS()
class EXTRACTIONSHOOTER14_API UEscapeProgressWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void InitializePrivateProperties(ATestGameState* GS, float InStartServerTime, float InDurationTime);
	
	void CloseEscapeProgressWidget();
	UFUNCTION()
	void OnFadeInAnimation_Finished();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> EscapeProgressBar;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StartTimeText;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DurationTimeText;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeInAnimation;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> TextAnimation;
	
	UFUNCTION()
	void UpdateEscapeProgressBar_Text();
private:
	TObjectPtr<ATestGameState> OwningGameState;
	float OverLappedTime;
	float DurationTime;
	
	FTimerHandle EscapeTimerHandle;
};
