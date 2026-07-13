#include "ES_TitleWidgetHUD.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Game/ES_SessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Slate/SGameLayerManager.h"


void UES_TitleWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
// ===================== Session ====================	
// ===================== Session ====================	
	
	UGameInstance* GameInstance =
		GetGameInstance();

	if (!IsValid(GameInstance))
	{
		return;
	}

	SessionSubsystem =
		GameInstance
			->GetSubsystem<UES_SessionSubsystem>();

	if (!IsValid(SessionSubsystem))
	{
		return;
	}

	SessionSubsystem
		->OnConnectionStateChanged
		.AddUniqueDynamic(
			this,
			&ThisClass::OnConnectionStateChanged
		);

	SessionSubsystem
		->OnConnectionCompleted
		.AddUniqueDynamic(
			this,
			&ThisClass::OnConnectionCompleted
		);
	
// ===================== Session ====================	
// ===================== Session ====================	
	
	APlayerController* Player = GetOwningPlayer();
	if (!IsValid(Player)) return;
	
	if (!Player->IsLocalController()) return;
	
	PlayAnimation(BlackFadeOut);
	PlayAnimation(TempSideMoveAnimation_PlB);
	PlayAnimation(TempSideMoveAnimation_SeB);
	PlayAnimation(TempSideMoveAnimation_EnB);

	FWidgetAnimationDynamicEvent Event;
	Event.BindDynamic(this, &ThisClass::OnFadeInAnimation_Back_Finished);
	BindToAnimationFinished(BlackFadeOut, Event);
	
	PlayButton = Cast<UButton>(PlayButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
	UButton* ButtonSet = Cast<UButton>(SettingsButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
	UButton* ButtonExit = Cast<UButton>(ExitButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
	PlayButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedPlayButton);
	ButtonSet->OnClicked.AddDynamic(this, &ThisClass::OnClickedSetutton);
	ButtonExit->OnClicked.AddDynamic(this, &ThisClass::OnClickedExitButton);
}

void UES_TitleWidgetHUD::NativeDestruct()
{
	Super::NativeDestruct();
	
	UnbindAllFromAnimationFinished(BlackFadeOut);
	
}

void UES_TitleWidgetHUD::OnClickedPlayButton()
{
	if (!IsValid(SessionSubsystem))
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("[TitleWidget] SessionSubsystem is invalid.")
		);
	
		return;
	}
	
	if (SessionSubsystem->IsConnecting())
	{
		return;
	}
	
	if (IsValid(PlayButton))
	{
		PlayButton->SetIsEnabled(false);
	}
	
	// SessionSubsystem->FindAndJoinDedicatedServer();
	
	
// ========================== Session ===========================
// ========================== Session ===========================
	
	 const FName IPString = FName(*IPTextBlock->GetText().ToString());
	 UGameplayStatics::OpenLevel(GetWorld(), IPString);
	// APlayerController* PlayerControllerES = GetOwningPlayer();
	// if (!IsValid(PlayerControllerES))
	// {
	// 	return;
	// }
	//
	// PlayerControllerES->ClientTravel(
	// 	TEXT("127.0.0.1:7777"),
	// 	ETravelType::TRAVEL_Absolute
	// );
}

void UES_TitleWidgetHUD::OnClickedSetutton()
{
	
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
		UUserWidget* LetterWidgetInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), LetterWidget);
		if (!IsValid(LetterWidgetInstance)) return;
		
		UTextBlock* Text =
		Cast<UTextBlock>(LetterWidgetInstance->GetWidgetFromName(TEXT("TextLetter")));
		
		Text->SetText(FText::FromString(FString::Chr(TitleName[TimerCount])));
		UHorizontalBoxSlot* HorizontalSlot = TitleHorizontal->AddChildToHorizontalBox(LetterWidgetInstance);
		LetterWidgetInstance->SetRenderTranslation(FVector2D(-5.f * TimerCount, 0.f));
		TimerCount++;
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(TitleLetterTimerHandle);
	}
}

void UES_TitleWidgetHUD::OnConnectionStateChanged(
	const FString& StateMessage
)
{
	if (IsValid(ConnectionStateText))
	{
		ConnectionStateText->SetText(
			FText::FromString(StateMessage)
		);
	}
}

void UES_TitleWidgetHUD::OnConnectionCompleted(
	bool bWasSuccessful
)
{
	if (bWasSuccessful)
	{
		return;
	}

	if (IsValid(PlayButton))
	{
		PlayButton->SetIsEnabled(true);
	}
}