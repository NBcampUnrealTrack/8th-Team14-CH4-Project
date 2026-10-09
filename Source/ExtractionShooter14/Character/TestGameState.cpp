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


void ATestGameState::StartRankUpdateTimer()
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(RankUpdateTimerHandle);

		GetWorldTimerManager().SetTimer(
			RankUpdateTimerHandle,
			this,
			&ThisClass::UpdateRankIfDirty,
			0.1f,
			true
		);
	}
}

void ATestGameState::StopRankUpdateTimer()
{
	if (HasAuthority())
	{
		GetWorldTimerManager().ClearTimer(RankUpdateTimerHandle);
		//bMarkRank = false;
	}
}

void ATestGameState::UpdateRankIfDirty()
{
	if (HasAuthority())
	{
		//bMarkRank = false;
		CalculatePlayerRank();
	}
}

void ATestGameState::CalculatePlayerRank()
{
	if (HasAuthority())
	{
		if (GameStatePhaseTimerData.GameStateEnum == EGameState::Playing)
		{
			// score changes every tick (survive time), rank changes only sometimes
			// > replicate only when rank changed
			bool bRankChanged = false;

			TArray<APlayerCharacterState*> CopyPlayerStates;

			for (auto State : LobbyPlayerStates)
			{
				if (!IsValid(State)) continue;
				if (State->GetIsParticipatedInThisRound())
				{
					// dead player survive time stops
					if (State->GetPlayerStagePlace() == EPlayerPlace::Stage && !State->GetbIsDead())
					{
						State->UpdateSurviveTime();
					}

					CopyPlayerStates.Add(State);
				}
			}

			// sort by latest(working) score, not last committed score
			CopyPlayerStates.Sort(
				[](const APlayerCharacterState& A, const APlayerCharacterState& B)
				{
					return A.GetPlayerWorkingCombatInfo().Score > B.GetPlayerWorkingCombatInfo().Score;
				});

			int32 CurrentRank = 1;

			for (int32 i = 0; i < CopyPlayerStates.Num(); ++i) // same score = 1, 1rank 
			{
				APlayerCharacterState* CurrentPS = CopyPlayerStates[i];
				if (!IsValid(CurrentPS)) continue;

				if (i > 0)
				{
					APlayerCharacterState* PrevPS = CopyPlayerStates[i - 1];

					if (IsValid(PrevPS) &&
						CurrentPS->GetPlayerWorkingCombatInfo().Score < PrevPS->GetPlayerWorkingCombatInfo().Score)
					{
						CurrentRank = i + 1;
					}
				}

				// true only when this player's rank changed (commit > replicate)
				if (CurrentPS->SetPlayerCombatInfo_Rank(CurrentRank))
				{
					bRankChanged = true;
				}
			}

			if (bRankChanged)
			{
				OnChangedPlayerStates.Broadcast();
			}
		}
	}
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
