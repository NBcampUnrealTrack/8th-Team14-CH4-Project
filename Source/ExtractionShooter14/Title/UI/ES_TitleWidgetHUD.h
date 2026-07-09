#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_TitleWidgetHUD.generated.h"

class UHorizontalBox;
class UTextBlock;
class UImage;
class UButton;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_TitleWidgetHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UFUNCTION(BlueprintCallable)
	void OnClickedPlayButton();
	UFUNCTION(BlueprintCallable)
	void OnClickedExitButton();
	
	UFUNCTION()
	void OnFadeInAnimation_Back_Finished();
public:
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UButton> PlayinButton;
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UButton> ExitoutButton;
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	// TObjectPtr<UUserWidget> WBP_TitleMenu;
	
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> IPTextBlock;
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> BlackFadeOut;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|UI")
	FString TitleName = "DedicatedX";
	
	FTimerHandle StartDelayTimer;
	FTimerHandle TitleLetterTimerHandle;
	int32 TimerCount = 0;
	void CreateLetterWidget();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UHorizontalBox> TitleHorizontal;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|UI")
	TSubclassOf<UUserWidget> LetterWidget; 
};
