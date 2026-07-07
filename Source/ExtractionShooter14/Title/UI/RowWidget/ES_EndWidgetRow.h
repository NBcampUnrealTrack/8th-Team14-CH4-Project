#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_EndWidgetRow.generated.h"

class UTextBlock;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_EndWidgetRow : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct();
	virtual void NativeDestruct();
	
	void SetTitle(const FText& Title, const FText& Value);
	
	UFUNCTION()
	void OnFadeUpAnimation_Tit_Finished();
	UFUNCTION()
	void OnFadeUpAnimation_Dir_Finished();
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InfoCategory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InfoValue;
	
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeUpAnimation_Tit;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeUpAnimation_Dir;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> FadeUpAnimation_Val;
	
};
