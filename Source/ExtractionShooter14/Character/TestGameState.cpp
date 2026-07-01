#include "TestGameState.h"
#include "Net/UnrealNetwork.h"
void ATestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInGameCount_GS);
}

void ATestGameState::SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount)
{
	PlayerInGameCount_GS = InNewPlayerInGameCount;
}

int32 ATestGameState::GetPlayerInGameCount_GS() const
{
	return PlayerInGameCount_GS;
}

void ATestGameState::OnRep_PlayerInGameCount_GS()
{
	OnChangedPlayerInGameCount.Broadcast(PlayerInGameCount_GS);
}
