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

void ATestGameState::SetGameStatePhaseTimerData(FPhaseTimerData InPhaseTimerData)
{
	if (HasAuthority())
	{
		GameStatePhaseTimerData.GameStateEnum = InPhaseTimerData.GameStateEnum;
		GameStatePhaseTimerData.PhaseStartServerTime = InPhaseTimerData.PhaseStartServerTime;
		GameStatePhaseTimerData.PhaseDuration = InPhaseTimerData.PhaseDuration;
	
		OnRep_GameStatePhaseTimerData();
	}
}

FPhaseTimerData ATestGameState::GetGameStatePhaseTimerData() const
{
	return GameStatePhaseTimerData;
}

float ATestGameState::GetRemainTime() const
{
	float CurrentServerTime = GetServerWorldTimeSeconds();
	// total timeLength - (now total time(ing) - start total time(stop)) 
	return FMath::Max(
		0.f,
		GameStatePhaseTimerData.PhaseDuration -
		(CurrentServerTime - GameStatePhaseTimerData.PhaseStartServerTime)
		);
}

void ATestGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	if (HasAuthority())
	{
		APlayerCharacterState* PlayerStateES = Cast<APlayerCharacterState>(PlayerState);
	
		if (!IsValid(PlayerStateES)) return;
		if (PlayerStateES->OnPlayerCombatInfoChanged.IsBoundToObject(this)) return;
	
		PlayerStateES->OnPlayerCombatInfoChanged.AddUObject(this, &ThisClass::CalculatePlayerRank);
		CalculatePlayerRank();
	}
}

void ATestGameState::RemovePlayerState(APlayerState* PlayerState)
{
	if (HasAuthority())
	{
		APlayerCharacterState* PlayerStateES = Cast<APlayerCharacterState>(PlayerState);
		if (!IsValid(PlayerStateES)) return;
	
		PlayerStateES->OnPlayerCombatInfoChanged.RemoveAll(this);
	}
	Super::RemovePlayerState(PlayerState);
	
	if (HasAuthority())
	{
		CalculatePlayerRank();
	}
}

void ATestGameState::CalculatePlayerRank()
{
	if (HasAuthority())
	{
		if (GameStatePhaseTimerData.GameStateEnum == EGameState::Playing)
		{
			TArray<APlayerCharacterState*> CopyPlayerStates = LobbyPlayerStates;
			
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
