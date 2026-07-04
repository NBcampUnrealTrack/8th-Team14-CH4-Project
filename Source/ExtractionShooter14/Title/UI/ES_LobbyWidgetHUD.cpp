#include "ES_LobbyWidgetHUD.h"

#include "Character/MyPlayerController.h"
#include "Character/PlayerCharacterState.h"
#include "Character/TestGameState.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Title/UI/ES_LobbyUserIconWidget.h"
void UES_LobbyWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	SetIsFocusable(true);
	
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
	}
	
	if (!IsValid(OwningPlayerState)) return;
	if (!PlayerinfoChangedHandle.IsValid())
	{
		PlayerinfoChangedHandle = OwningPlayerState->OnPlayerDataChanged.AddUObject(this, &ThisClass::RefreshPlayerInfo);
		RefreshPlayerInfo();
	}
	//AddUserIcon();
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
		OwningPlayerState->OnPlayerDataChanged.Remove(PlayerinfoChangedHandle);
		PlayerinfoChangedHandle.Reset();
	}
}

FReply UES_LobbyWidgetHUD::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{

	if (InKeyEvent.GetKey() == EKeys::LeftControl)
	{
		OwningPlayerController->OpenOptionUI();
		// FReply를 반환하지않으면 controller에게 까지 입력이 전달됨
		return FReply::Handled();
	}
	
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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

void UES_LobbyWidgetHUD::SetLobbyCountPlayer(const int32 InNewPlayer)
{
	LobbyCountPlayer->SetText(FText::AsNumber(InNewPlayer));
}

void UES_LobbyWidgetHUD::RefreshPlayerInfo()
{
	if (!IsValid(OwningPlayerState)) return;
	if (!IsValid(LobbyTotalGold)) return;
	
	UTextBlock* GoldText = Cast<UTextBlock>(
		LobbyTotalGold->GetWidgetFromName(TEXT("TXT_Gold_Text"))
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

void UES_LobbyWidgetHUD::CachedPlayerController(AMyPlayerController* InPlayerController)
{
	OwningPlayerController = InPlayerController;
}
