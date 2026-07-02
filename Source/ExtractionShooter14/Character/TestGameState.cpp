#include "TestGameState.h"
#include "Net/UnrealNetwork.h"
void ATestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInGameCount_GS);
	DOREPLIFETIME(ThisClass, LobbyPlayerStates);
}

void ATestGameState::SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount)
{
	// in server
	PlayerInGameCount_GS = InNewPlayerInGameCount;
}

int32 ATestGameState::GetPlayerInGameCount_GS() const
{
	return PlayerInGameCount_GS;
}

void ATestGameState::AddLobbyPlayerStates(APlayerCharacterState* InNewPlayerState)
{
	// in server
	LobbyPlayerStates.AddUnique(InNewPlayerState);
	//SetPlayerInGameCount_GS(LobbyPlayerStates.Num());

	OnChangedPlayerStates.Broadcast();
}

void ATestGameState::RemoveLobbyPlayerStates(APlayerCharacterState* InOutPlayerState)
{
	// in server
	LobbyPlayerStates.Remove(InOutPlayerState);
}

const TArray<APlayerCharacterState*>& ATestGameState::GetLobbyPlayerStates() const
{
	return LobbyPlayerStates;
}



void ATestGameState::OnRep_PlayerInGameCount_GS()
{
	OnChangedPlayerInGameCount.Broadcast(PlayerInGameCount_GS);
}

void ATestGameState::OnRep_LobbyPlayerStates()
{
	// uobject
	OnChangedPlayerStates.Broadcast();
}
