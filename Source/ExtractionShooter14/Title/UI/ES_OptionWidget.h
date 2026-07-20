#pragma once

#include "CoreMinimal.h"
#include "WidgetBase/ClosableWidgetBase.h"
#include "ES_OptionWidget.generated.h"

class UButton;

UCLASS()
class EXTRACTIONSHOOTER14_API UES_OptionWidget : public UClosableWidgetBase
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	UFUNCTION()
	void OnClicked_GotoTitle();
	UFUNCTION()
	void OnClicked_Settings();
	UFUNCTION()
	void OnClicked_Resume();
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ResumButtonWidget;
	UPROPERTY()
	TObjectPtr<UButton> ResumButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> SettingsButtonWidget;
	UPROPERTY()
	TObjectPtr<UButton> SettingsButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> GoTitleButtonWidget;
	UPROPERTY()
	TObjectPtr<UButton> GoTitleButton;
};
