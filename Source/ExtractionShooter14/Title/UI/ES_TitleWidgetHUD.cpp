#include "ES_TitleWidgetHUD.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Slate/SGameLayerManager.h"


void UES_TitleWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	APlayerController* Player = GetOwningPlayer();
	if (!IsValid(Player)) return;
	
	if (!Player->IsLocalController()) return;
	
	PlayAnimation(BlackFadeOut);
	
	// PlayinButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedPlayButton);
	// ExitoutButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedExitButton);
	// FWidgetAnimationDynamicEvent Event;
	// Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Back_Finished);
	// BindToAnimationFinished(BlackFadeOut, Event);
	
	GetWorld()->GetTimerManager().SetTimer(
		StartDelayTimer,
		[this]()
		{
			OnFadeInAnimation_Back_Finished();
		},
		1.f,
		false);
}

void UES_TitleWidgetHUD::NativeDestruct()
{
	Super::NativeDestruct();
	
	//UnbindAllFromAnimationFinished(BlackFadeOut);
	
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

void UES_TitleWidgetHUD::OnFadeInAnimation_Back_Finished()
{
	GetWorld()->GetTimerManager().SetTimer(
		TitleLetterTimerHandle,
		this,
		&ThisClass::CreateLetterWidget,
		0.2f,
		true);
}

void UES_TitleWidgetHUD::CreateLetterWidget()
{
	if (TimerCount < TitleName.Len())
	{
		if (!IsValid(LetterWidget)) return;
		UUserWidget* LetterWidgetInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), LetterWidget);
		if (!IsValid(LetterWidgetInstance)) return;
		
		UTextBlock* Text =
		Cast<UTextBlock>(LetterWidgetInstance->GetWidgetFromName(TEXT("TextLetter")));
		
		Text->SetText(FText::FromString(FString::Chr(TitleName[TimerCount])));
		UHorizontalBoxSlot* HorizontalSlot = TitleHorizontal->AddChildToHorizontalBox(LetterWidgetInstance);
		LetterWidgetInstance->SetRenderTranslation(FVector2D(-15.f * TimerCount, 0.f));
		TimerCount++;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TitleLetterTimerHandle);
	}
}
