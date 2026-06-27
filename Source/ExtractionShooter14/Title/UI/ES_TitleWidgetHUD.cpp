#include "ES_TitleWidgetHUD.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


void UES_TitleWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	APlayerController* Player = GetOwningPlayer();
	if (!IsValid(Player)) return;
	
	if (!Player->IsLocalController()) return;
	
	PlayAnimation(BlackFadeOut);
	
	// PlayinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedPlayButton);
	// ExitoutButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedExitButton);
	
}

void UES_TitleWidgetHUD::OnClickedPlayButton()
{
	APlayerController* Player = GetOwningPlayer();
	if (!IsValid(Player)) return;
	
	FInputModeGameOnly InputMode;
	Player->SetInputMode(InputMode);
	Player->bShowMouseCursor = false;
	
	UWidgetLayoutLibrary::RemoveAllWidgets(this);
	
	const FName IPString = FName(*IPTextBlock->GetText().ToString());
	UGameplayStatics::OpenLevel(GetWorld(), IPString);
}

void UES_TitleWidgetHUD::OnClickedExitButton()
{
	APlayerController* Player = GetOwningPlayer();
	if (!IsValid(Player)) return;
	
	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		Player,
		EQuitPreference::Quit,
		false);
}
