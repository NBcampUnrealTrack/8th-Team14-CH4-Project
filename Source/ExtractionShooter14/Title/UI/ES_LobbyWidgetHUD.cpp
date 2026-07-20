#include "ES_LobbyWidgetHUD.h"

#include "Character/MyPlayerController.h"
#include "Character/PlayerCharacterState.h"
#include "Character/TestGameState.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Title/UI/ES_LobbyUserIconWidget.h"
void UES_LobbyWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (IsValid(ReadyButtonWidget))
	{
		UButton* ReadyButton = Cast<UButton>(ReadyButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
		if (IsValid(ReadyButton))
		{
			ReadyButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedReadyButton);
			ReadyButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedReadyButton);
		}
	}
	if (IsValid(ShopButtonWidget))
	{
		UButton* ShopButton = Cast<UButton>(ShopButtonWidget->GetWidgetFromName(TEXT("PlayButton")));
		if (IsValid(ShopButton))
		{
			ShopButton->OnClicked.RemoveDynamic(this, &ThisClass::OnClickedShopButton);
			ShopButton->OnClicked.AddDynamic(this, &ThisClass::OnClickedShopButton);
		}
	}
	
	ATestGameState* GameStateES = Cast<ATestGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!IsValid(GameStateES)) return;
	OwningGameState = GameStateES;
	// count textblock
	if (false == GameStateES->OnChangedPlayerInGameCount.IsAlreadyBound(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer))
	{
		GameStateES->OnChangedPlayerInGameCount.AddDynamic(this, &ThisClass::SetLobbyCountPlayer);
		SetLobbyCountPlayer(GameStateES->GetPlayerInGameCount_GS());
		PlayerStateChangedHandle = GameStateES->OnChangedPlayerStates.AddUObject(this, &ThisClass::RefreshPlayerUserIcon);
		RefreshPlayerUserIcon();
		PhaseTimerChangedHandle = GameStateES->OnChangedGameStatePhaseTimerData.AddUObject(this, &UES_LobbyWidgetHUD::HandlePhaseTimerChanged);
		HandlePhaseTimerChanged();
	}
	
	if (!IsValid(OwningPlayerState)) return;
	if (!PlayerinfoChangedHandle.IsValid())
	{
		PlayerinfoChangedHandle = OwningPlayerState->OnPlayerInfoChanged.AddUObject(this, &ThisClass::RefreshPlayerInfo);
		RefreshPlayerInfo();
	}
}

void UES_LobbyWidgetHUD::NativeDestruct()
{
	Super::NativeDestruct();
	
	ATestGameState* GameStateES = Cast<ATestGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!IsValid(GameStateES)) return;
	
	// count textblock
	if (true == GameStateES->OnChangedPlayerInGameCount.IsAlreadyBound(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer))
	{
		GameStateES->OnChangedPlayerInGameCount.RemoveDynamic(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer);
		GameStateES->OnChangedPlayerStates.Remove(PlayerStateChangedHandle);
	}
	
	if (PlayerinfoChangedHandle.IsValid() && IsValid(OwningPlayerState))
	{
		OwningPlayerState->OnPlayerInfoChanged.Remove(PlayerinfoChangedHandle);
		PlayerinfoChangedHandle.Reset();
	}
	
	if (PhaseTimerChangedHandle.IsValid() && IsValid(OwningGameState))
	{
		OwningGameState->OnChangedGameStatePhaseTimerData.Remove(PhaseTimerChangedHandle);
		PhaseTimerChangedHandle.Reset();
	}
	
	GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
	
}

void UES_LobbyWidgetHUD::RefreshPlayerUserIcon()
{
	//if (!IsValid(OwningPlayerState)) return;
	
	if (!IsValid(OwningPlayerState))
	{
		OwningPlayerState = GetOwningPlayer()->GetPlayerState<APlayerCharacterState>();
	}
	if (!IsValid(OwningPlayerState)) return;
	if (!IsValid(OwningGameState)) return;
	if (!IsValid(UserIconClass)) return;
	
	PlayerIconHorizontal->ClearChildren();
	
	for ( auto EachPlayerState : OwningGameState->GetLobbyPlayerStates())
	{
		if (!IsValid(EachPlayerState)) continue;
		
		// APlayerCharacterState* EachPlayerStateES = Cast<APlayerCharacterState>(EachPlayerState);
		// if (!IsValid(EachPlayerStateES)) continue;
		
		UES_LobbyUserIconWidget* UserIconInstance =
			CreateWidget<UES_LobbyUserIconWidget>(GetOwningPlayer(), UserIconClass);
		
		UserIconInstance->CachedPlayerCharacterState(EachPlayerState);
		PlayerIconHorizontal->AddChild(UserIconInstance);
		
		UserIconInstance->RefreshReadyIcon(EachPlayerState->IsPlayerReady());
	}
}

void UES_LobbyWidgetHUD::OnClickedReadyButton()
{
	if (!IsValid(OwningPlayerState)) return;
	
	OwningPlayerState->Server_SetPlayerReady(!OwningPlayerState->IsPlayerReady());
}

void UES_LobbyWidgetHUD::OnClickedShopButton()
{
	// controller call
	if (!IsValid(OwningPlayerController)) return;
	
	OwningPlayerController->ToggleShopWidget();
}

void UES_LobbyWidgetHUD::SetLobbyCountPlayer(const int32 InNewPlayer)
{
	LobbyCountPlayer->SetText(FText::AsNumber(InNewPlayer));
}

void UES_LobbyWidgetHUD::RefreshPlayerInfo()
{
	UE_LOG(
	LogTemp,
	Warning,
	TEXT("[RefreshPlayerInfo] Widget=%s PS=%s Gold=%d InViewport=%d"),
	*GetNameSafe(this),
	*GetNameSafe(OwningPlayerState),
	IsValid(OwningPlayerState)
		? OwningPlayerState->GetPlayerInfo().GoldStorage
		: -1,
	IsInViewport()
);
	
	if (!IsValid(OwningPlayerState)) return;
	if (!IsValid(LobbyTotalGold)) return;
	
	UTextBlock* GoldText = Cast<UTextBlock>(
		LobbyTotalGold->GetWidgetFromName(TEXT("TXT_Gold_Text"))
	);
	UE_LOG(
	LogTemp,
	Error,
	TEXT("[RefreshPlayerInfo] GoldText Invalid Widget=%s"),
	*GetNameSafe(this)
);
	
	const FPlayerDatas& NewPlayerData = OwningPlayerState->GetPlayerInfo();
	
	GoldText->SetText(FText::AsNumber(NewPlayerData.GoldStorage));
	PlayerNickName->SetText(FText::FromString(NewPlayerData.PlayerNickName));
}

void UES_LobbyWidgetHUD::AddUserIcon()
{
	// if (!IsValid(UserIconClass)) return;
	// UserIconInstance = CreateWidget<UES_LobbyUserIconWidget>(this, UserIconClass);
	// if (!IsValid(UserIconInstance)) return;
	//
	// PlayerIconHorizontal->AddChild(UserIconInstance);
	// UserIconInstance->CachedPlayerCharacterState(OwningPlayerState);
}

void UES_LobbyWidgetHUD::CachedPlayerState(APlayerCharacterState* InPlayerState)
{
	OwningPlayerState = InPlayerState;
}

void UES_LobbyWidgetHUD::CachedGameState(ATestGameState* InTestGameState)
{
	OwningGameState = InTestGameState;
}

void UES_LobbyWidgetHUD::CachedPlayerController(AMyPlayerController* InPlayerController)
{
	OwningPlayerController = InPlayerController;
}

void UES_LobbyWidgetHUD::HandlePhaseTimerChanged()
{
	if (!IsValid(OwningGameState)) return;

	const FPhaseTimerData& TimerData =
		OwningGameState->GetGameStatePhaseTimerData();
	if (TimerData.GameStateEnum == EGameState::None ||
		TimerData.PhaseDuration <= 0.f ||
		TimerData.PhaseEndServerTime <= 0.f)
	{
		return;
	}

	UpdateTimerText();
	GetWorld()->GetTimerManager().ClearTimer(PhaseTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&ThisClass::UpdateTimerText,
		0.2f,
		true
	);
}

void UES_LobbyWidgetHUD::UpdateTimerText()
{
	if (!IsValid(OwningGameState)) return;

	float RemainTime = OwningGameState->GetRemainTime(); // 100seconds, 50seconds
	int32 TotalSeconds = FMath::CeilToInt(RemainTime); // float to int

	int32 Minutes =	TotalSeconds / 60; // 100seconds / 60 = 1
	int32 Seconds =	TotalSeconds % 60; // 100seconds % 60 = 40

	LobbyTimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"),Minutes,Seconds)));
	
	ApplyTextColorAnimation(RemainTime);
}

void UES_LobbyWidgetHUD::ApplyTextColorAnimation(float LeftTime)
{
	if (!IsValid(OwningGameState)) return;
	
	const FPhaseTimerData& TimerData =
		OwningGameState->GetGameStatePhaseTimerData();

	if (TimerData.PhaseDuration <= 0.f) return;

	const EGameState GamePhase = TimerData.GameStateEnum;

	const float PassedTime =
		OwningGameState->GetServerWorldTimeSeconds()
		- TimerData.PhaseStartServerTime;

	const float Ratio = PassedTime / TimerData.PhaseDuration;
	
	if (GamePhase == EGameState::LobbyWaiting)
	{
		if (LeftTime > 5.f)
		{
			LobbyTimerText->SetColorAndOpacity(FColor::White);
		}
		else if (LeftTime <= 5.f)
		{
			LobbyTimerText->SetColorAndOpacity(FColor::Red);
		}
	}
	else if (GamePhase == EGameState::Playing)
	{
		if (Ratio < 0.2f)
		{
			LobbyTimerText->SetColorAndOpacity(FColor::Green);
		}
		else
		{
			if (LeftTime > 5.f)
			{
				LobbyTimerText->SetColorAndOpacity(FColor::White);
			}
			else if (LeftTime <= 5.f)
			{
				LobbyTimerText->SetColorAndOpacity(FColor::Red);
			}
		}
	}
}