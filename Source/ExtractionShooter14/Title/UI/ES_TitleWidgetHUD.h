#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ES_TitleWidgetHUD.generated.h"

class UEditableText;
class UVerticalBox;
class UES_SessionSubsystem;
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
	void OnClickedSetutton();
	UFUNCTION(BlueprintCallable)
	void OnClickedExitButton();
	
	UFUNCTION()
	void OnFadeInAnimation_Back_Finished();
	
	UFUNCTION()
	void OnConnectionStateChanged(const FString& StateMessage);
	UFUNCTION()
	void OnConnectionCompleted(bool bWasSuccessful);
	
public:
	UPROPERTY()
	TObjectPtr<UButton> PlayButton;
	
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UUserWidget> PlayButtonWidget;
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UUserWidget> SettingsButtonWidget;
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UUserWidget> ExitButtonWidget;
	
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> TempSideMoveAnimation_PlB;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> TempSideMoveAnimation_SeB;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> TempSideMoveAnimation_EnB;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> BlackFadeOut;
	
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> IPTextBlock;
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UEditableText> IPEditableText;
	UPROPERTY(BlueprintReadOnly, Category = "Title|UI", meta = (BindWidget))
	TObjectPtr<UTextBlock> ConnectionStateText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|UI")
	FString TitleName1 = "The";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|UI")
	FString TitleName2 = "Lost";
	
	FTimerHandle TitleUpperLetterTimerHandle;
	FTimerHandle TitleDownLetterTimerHandle;
	int32 TimerCount = 0;
	void CreateLetterWidget();
	void CreateDownLetterWidget();
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> TitleHorizontal;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UHorizontalBox> TitleHorizontal_1;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> ButtonsVerticalBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title|UI")
	TSubclassOf<UUserWidget> LetterWidget; 
private:
	UPROPERTY()
	TObjectPtr<UES_SessionSubsystem> SessionSubsystem;
};
