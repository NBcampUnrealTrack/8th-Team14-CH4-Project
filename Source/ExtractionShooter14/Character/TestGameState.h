#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TestGameState.generated.h"

class APlayerCharacterState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChanedPlayerInGameCount, int32, InNewPlayerCount);
DECLARE_MULTICAST_DELEGATE(FChangedLobbyPlayerStates);
DECLARE_MULTICAST_DELEGATE(FChangedGameStatePhaseTimerData);

#pragma region Struct&Enum
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
	float PhaseStartServerTime = 0.f;

	UPROPERTY()
	float PhaseDuration = 0.f;
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
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount);
	int32 GetPlayerInGameCount_GS() const;
	
	void AddLobbyPlayerStates(APlayerCharacterState* InNewPlayerState);
	void RemoveLobbyPlayerStates(APlayerCharacterState* InOutPlayerState);
	void ForceAllPlayersReady();
	
	const TArray<APlayerCharacterState*>& GetLobbyPlayerStates() const;
	
	UFUNCTION()
	void OnRep_GameStatePhaseTimerData();
	void SetGameStatePhaseTimerData(FPhaseTimerData InPhaseTimerData);
	FPhaseTimerData GetGameStatePhaseTimerData() const;
	
	float GetRemainTime() const;
	
#pragma region Rank
	public:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	void CalculatePlayerRank();
#pragma endregion 
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
};

