#include "ES_LobbyUserIconWidget.h"

#include "Character/PlayerCharacterState.h"
#include "Components/Image.h"

void UES_LobbyUserIconWidget::RefreshReadyIcon(uint8 bisready)
{
	if (bisready)
	{
		ReadyIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ReadyIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UES_LobbyUserIconWidget::CachedPlayerCharacterState(APlayerCharacterState* PlayerCharacterState)
{
	OwningPlayerState = PlayerCharacterState;
	
	if (!PlayerCharacterState) return;
	
	OwningPlayerState->OnPlayerReadyChanged.AddDynamic(this, &ThisClass::RefreshReadyIcon);
	RefreshReadyIcon(OwningPlayerState->IsPlayerReady());
}
