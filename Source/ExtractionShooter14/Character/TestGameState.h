#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TestGameState.generated.h"

class APlayerCharacterState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChanedPlayerInGameCount, int32, InNewPlayerCount);
DECLARE_MULTICAST_DELEGATE(FChangedLobbyPlayerStates);
DECLARE_MULTICAST_DELEGATE(FChangedGameStatePhaseTimerData);
DECLARE_MULTICAST_DELEGATE(FChangedTopPlayer);

#pragma region Struct&Enum
// current 1st place, replicated to every client (one struct, not per player)
USTRUCT(BlueprintType)
struct FTopPlayerInfo
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PlayerId = INDEX_NONE;

	UPROPERTY()
	int32 Score = 0;
};

UENUM()
enum class EGameState : uint8
{
	None,
	LobbyWaiting,
	Playing
};

USTRUCT(BlueprintType)
struct FPhaseTimerData
{
	GENERATED_BODY()

	UPROPERTY()
	EGameState GameStateEnum = EGameState::None;

	UPROPERTY()
	double PhaseStartServerTime = 0.0;

	UPROPERTY()
	float PhaseDuration = 0.f;
	
	UPROPERTY()
	double PhaseEndServerTime = 0.0;
};
#pragma endregion 

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameState : public AGameStateBase
{
	GENERATED_BODY()
public:	
	FChanedPlayerInGameCount OnChangedPlayerInGameCount;
	FChangedLobbyPlayerStates OnChangedPlayerStates;
	FChangedGameStatePhaseTimerData OnChangedGameStatePhaseTimerData;
	FChangedTopPlayer OnChangedTopPlayer;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount);
	int32 GetPlayerInGameCount_GS() const;
	
	void AddLobbyPlayerStates(APlayerCharacterState* InNewPlayerState);
	void RemoveLobbyPlayerStates(APlayerCharacterState* InOutPlayerState);
	
	const TArray<APlayerCharacterState*>& GetLobbyPlayerStates() const;
	
	UFUNCTION()
	void OnRep_GameStatePhaseTimerData();
	void SetGameStatePhaseTimerData(EGameState NewState, float Duration);
	FPhaseTimerData GetGameStatePhaseTimerData() const;
	
	float GetRemainTime() const;
	// engine client server time lags by one-way latency : add half RTT (server returns as is)
	double GetSyncedServerTimeSeconds() const;

	void SetTopPlayer(int32 InPlayerId, int32 InScore); // server
	const FTopPlayerInfo& GetTopPlayer() const { return TopPlayer; }
	FString GetTopPlayerNickName() const;
	UFUNCTION()
	void OnRep_TopPlayer();
	
private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInGameCount_GS)
	int32 PlayerInGameCount_GS;
	UFUNCTION()
	void OnRep_PlayerInGameCount_GS();
	
	UPROPERTY(ReplicatedUsing=OnRep_LobbyPlayerStates)
	TArray<APlayerCharacterState*> LobbyPlayerStates;
	UFUNCTION()
	void OnRep_LobbyPlayerStates();
private:
	UPROPERTY(ReplicatedUsing=OnRep_GameStatePhaseTimerData)
	FPhaseTimerData GameStatePhaseTimerData;

	UPROPERTY(ReplicatedUsing=OnRep_TopPlayer)
	FTopPlayerInfo TopPlayer;
};

