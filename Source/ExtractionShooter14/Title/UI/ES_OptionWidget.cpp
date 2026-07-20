#include "ES_OptionWidget.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
#include "Character/MyPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UES_OptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(ResumButtonWidget))
	{
		ResumButton = 
			Cast<UButton>(ResumButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
		if (IsValid(ResumButton))
		{
			ResumButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_Resume);
			ResumButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Resume);
		}
	}
	
	if (IsValid(SettingsButtonWidget))
	{
		SettingsButton = 
			Cast<UButton>(SettingsButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
		if (IsValid(SettingsButton))
		{
			SettingsButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_Settings);
			SettingsButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked_Settings);
		}
	}
	
	if (IsValid(GoTitleButtonWidget))
	{
		GoTitleButton = 
			Cast<UButton>(GoTitleButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
		if (IsValid(GoTitleButton))
		{
			GoTitleButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_GotoTitle);
			GoTitleButton->OnClicked.AddDynamic(this, &ThisClass::OnClicked_GotoTitle);
		}
	}
}

void UES_OptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	
	if (IsValid(ResumButton))
	{
		if (ResumButton->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Resume))
		{
			ResumButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_Resume);
		}
	}
	if (IsValid(SettingsButton))
	{
		if (SettingsButton->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_Settings))
		{
			SettingsButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_Settings);
		}
	}
	if (IsValid(GoTitleButton))
	{
		if (GoTitleButton->OnClicked.IsAlreadyBound(this, &ThisClass::OnClicked_GotoTitle))
		{
			GoTitleButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClicked_GotoTitle);
		}
	}
}

void UES_OptionWidget::OnClicked_GotoTitle()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Title_Map"));
}

void UES_OptionWidget::OnClicked_Settings()
{
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (!IsValid(MyPC)) return;
	
	MyPC->MakeSettingWidget();
}

void UES_OptionWidget::OnClicked_Resume()
{
	AMyPlayerController* MyPC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (!IsValid(MyPC)) return;
	
	MyPC->OnPressedEscape();
}
