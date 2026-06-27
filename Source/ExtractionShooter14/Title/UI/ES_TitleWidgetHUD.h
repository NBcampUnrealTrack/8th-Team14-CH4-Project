#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_TitleWidgetHUD.generated.h"

class UTextBlock;
class UImage;
class UButton;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_TitleWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void OnClickedPlayButton();
	UFUNCTION(BlueprintCallable)
	void OnClickedExitButton();
public:
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UButton> PlayinButton;
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UButton> ExitoutButton;
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UUserWidget> WBP_TitleMenu;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> IPTextBlock;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> BlackFadeOut;
};
