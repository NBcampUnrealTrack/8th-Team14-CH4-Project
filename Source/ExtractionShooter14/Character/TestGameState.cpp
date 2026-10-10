#include "TestGameState.h"

#include "PlayerCharacterState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
void ATestGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInGameCount_GS);
	DOREPLIFETIME(ThisClass, LobbyPlayerStates);
	DOREPLIFETIME(ThisClass, GameStatePhaseTimerData);
	DOREPLIFETIME(ThisClass, TopPlayer);
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
		const double ServerNow = GetServerWorldTimeSeconds();
		
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
	if (GameStatePhaseTimerData.PhaseEndServerTime <= 0.0) return 0.f;

	const double ServerNow = GetSyncedServerTimeSeconds();

	return static_cast<float>(FMath::Max(
		0.0,
		GameStatePhaseTimerData.PhaseEndServerTime - ServerNow));
}

double ATestGameState::GetSyncedServerTimeSeconds() const
{
	const double ServerNow = GetServerWorldTimeSeconds();
	if (HasAuthority()) return ServerNow;

	// delta = (server time when sent) - (client time when received) : behind by one-way latency
	const UWorld* World = GetWorld();
	const APlayerController* LocalController = World ? World->GetFirstPlayerController() : nullptr;
	const APlayerState* LocalPlayerState = LocalController ? LocalController->PlayerState : nullptr;
	if (!IsValid(LocalPlayerState)) return ServerNow;

	// ping = round trip (ms, measured by engine) > one-way latency = half of it
	const double RoundTripSeconds = LocalPlayerState->GetPingInMilliseconds() / 1000.0;
	const double OneWayLatencySeconds = RoundTripSeconds * 0.5;

	return ServerNow + OneWayLatencySeconds;
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

void ATestGameState::SetTopPlayer(int32 InPlayerId, int32 InScore)
{
	if (!HasAuthority()) return;
	if (TopPlayer.PlayerId == InPlayerId && TopPlayer.Score == InScore) return;

	TopPlayer.PlayerId = InPlayerId;
	TopPlayer.Score = InScore;
	OnChangedTopPlayer.Broadcast(); // listen server host HUD (no OnRep on server)
}

FString ATestGameState::GetTopPlayerNickName() const
{
	for (APlayerState* EachPlayerState : PlayerArray)
	{
		if (!IsValid(EachPlayerState) || EachPlayerState->GetPlayerId() != TopPlayer.PlayerId) continue;

		if (APlayerCharacterState* PlayerStateES = Cast<APlayerCharacterState>(EachPlayerState))
		{
			return PlayerStateES->GetPlayerInfo().PlayerNickName;
		}
	}
	return FString();
}

void ATestGameState::OnRep_TopPlayer()
{
	OnChangedTopPlayer.Broadcast();
}
