#include "ES_LobbyWidgetHUD.h"

#include "Character/TestGameState.h"
#include "Components/HorizontalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Title/UI/ES_LobbyUserIconWidget.h"
void UES_LobbyWidgetHUD::NativeConstruct()
{
	Super::NativeConstruct();
	
	ATestGameState* GameStateES = Cast<ATestGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!IsValid(GameStateES)) return;
	
	if (false == GameStateES->OnChangedPlayerInGameCount.IsAlreadyBound(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer))
	{
		GameStateES->OnChangedPlayerInGameCount.AddDynamic(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer);
		SetLobbyCountPlayer(GameStateES->GetPlayerInGameCount_GS());
	}
}

void UES_LobbyWidgetHUD::NativeDestruct()
{
	Super::NativeDestruct();
	
	ATestGameState* GameStateES = Cast<ATestGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!IsValid(GameStateES)) return;
	
	if (true == GameStateES->OnChangedPlayerInGameCount.IsAlreadyBound(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer))
	{
		GameStateES->OnChangedPlayerInGameCount.RemoveDynamic(this, &UES_LobbyWidgetHUD::SetLobbyCountPlayer);
	}
}

void UES_LobbyWidgetHUD::OnClickedReadyButton()
{
}

void UES_LobbyWidgetHUD::SetLobbyCountPlayer(const int32 InNewPlayer)
{
	LobbyCountPlayer->SetText(FText::AsNumber(InNewPlayer));
}

void UES_LobbyWidgetHUD::AddUserIcon()
{
	if (!IsValid(UserIconClass)) return;
	UserIconInstance = CreateWidget<UES_LobbyUserIconWidget>(this, UserIconClass);
	if (!IsValid(UserIconInstance)) return;
	
	PlayerIconHorizontal->AddChild(UserIconInstance);
}
