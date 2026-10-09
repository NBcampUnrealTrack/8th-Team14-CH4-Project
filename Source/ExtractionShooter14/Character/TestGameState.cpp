#include "TestGameState.h"

#include "PlayerCharacterState.h"
#include "Net/UnrealNetwork.h"
void ATestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInGameCount_GS);
	DOREPLIFETIME(ThisClass, LobbyPlayerStates);
	DOREPLIFETIME(ThisClass, GameStatePhaseTimerData);
}

void ATestGameState::SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount)
{
	// in server
	PlayerInGameCount_GS = InNewPlayerInGameCount;
	OnChangedPlayerInGameCount.Broadcast(PlayerInGameCount_GS);
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
	OnChangedPlayerStates.Broadcast();
}

const TArray<APlayerCharacterState*>& ATestGameState::GetLobbyPlayerStates() const
{
	return LobbyPlayerStates;
}

void ATestGameState::OnRep_GameStatePhaseTimerData()
{ // waiting // playing //
	OnChangedGameStatePhaseTimerData.Broadcast();
}

void ATestGameState::SetGameStatePhaseTimerData(EGameState NewState, float Duration)
{
	if (HasAuthority())
	{
		const float ServerNow = GetServerWorldTimeSeconds();
		
		GameStatePhaseTimerData.GameStateEnum = NewState;
		GameStatePhaseTimerData.PhaseStartServerTime = ServerNow;
		GameStatePhaseTimerData.PhaseDuration = Duration;
		GameStatePhaseTimerData.PhaseEndServerTime = ServerNow + Duration;
		OnRep_GameStatePhaseTimerData();
	}
}

FPhaseTimerData ATestGameState::GetGameStatePhaseTimerData() const
{
	return GameStatePhaseTimerData;
}

float ATestGameState::GetRemainTime() const
{
	if (GameStatePhaseTimerData.PhaseEndServerTime <= 0.f) return 0.f;
	
	const float ServerNow = GetServerWorldTimeSeconds();
	
	return FMath::Max(
		0.f,
		GameStatePhaseTimerData.PhaseEndServerTime - ServerNow);
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
