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

void ATestGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	if (HasAuthority())
	{
		// APlayerCharacterState* PlayerStateES = Cast<APlayerCharacterState>(PlayerState);
		//
		// if (!IsValid(PlayerStateES)) return;
		// if (PlayerStateES->OnPlayerCombatInfoChanged.IsBoundToObject(this)) return;
		//
		// PlayerStateES->OnPlayerCombatInfoChanged.AddUObject(this, &ThisClass::CalculatePlayerRank);
		// CalculatePlayerRank();
	}
}

void ATestGameState::RemovePlayerState(APlayerState* PlayerState)
{
	// if (HasAuthority())
	// {
	// 	APlayerCharacterState* PlayerStateES = Cast<APlayerCharacterState>(PlayerState);
	// 	if (!IsValid(PlayerStateES)) return;
	//
	// 	PlayerStateES->OnPlayerCombatInfoChanged.RemoveAll(this);
	// }
	Super::RemovePlayerState(PlayerState);
	
	// if (HasAuthority())
	// {
	// 	CalculatePlayerRank();
	// }
}

// void ATestGameState::MarkRank()
// {
// 	if (HasAuthority())
// 	{
// 		bMarkRank = true;
// 	}
// }

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
			TArray<APlayerCharacterState*> CopyPlayerStates;
			
			for (auto State : LobbyPlayerStates)
			{
				if (State->GetIsParticipatedInThisRound())
				{
					if (State->GetPlayerStagePlace() == EPlayerPlace::Stage)
					{
						State->UpdateSurviveTime();
					}

					CopyPlayerStates.Add(State);
				} 
			}
			
			CopyPlayerStates.Sort(
				[](const APlayerCharacterState& A, const APlayerCharacterState& B)
				{
					return A.GetPlayerCombatInfo().Score > B.GetPlayerCombatInfo().Score;
				});
			// todo:: removeall
			
			
			int32 CurrentRank = 1;
		
			for (int32 i = 0; i < CopyPlayerStates.Num(); ++i)
			{
				APlayerCharacterState* CurrentPS = CopyPlayerStates[i];
				if (!IsValid(CurrentPS)) continue;
		
				if (i > 0)
				{
					APlayerCharacterState* PrevPS = CopyPlayerStates[i - 1];
		
					if (IsValid(PrevPS) &&
						CurrentPS->GetPlayerCombatInfo().Score < PrevPS->GetPlayerCombatInfo().Score)
					{
						CurrentRank = i + 1;
					}
				}
		
				CurrentPS->SetPlayerCombatInfo_Rank(CurrentRank);
			}
		}
		OnChangedPlayerStates.Broadcast();
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
