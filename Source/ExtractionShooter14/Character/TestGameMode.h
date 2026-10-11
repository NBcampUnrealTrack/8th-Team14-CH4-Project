#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

class AMyPlayerController;

DECLARE_MULTICAST_DELEGATE(FOnStageEnded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerLoggedOut, AMyPlayerController*);

struct FPhaseTimerData;
enum class EPlayerPlace : uint8;
class APlayerCharacterState;

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	
	FOnStageEnded OnStageEnded;
	// before the leaving controller is destroyed (clean up per-player state such as escape countdown)
	FOnPlayerLoggedOut OnPlayerLoggedOut;
	
public:
	ATestGameMode();
	virtual void BeginPlay() override;
	
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	int32 GetAvailableStartIndex() const;
	FTransform GetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void SetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController);
	void TimeOutReSetPlayer(AMyPlayerController* TargetController);
	void InitializePlayerStats(APlayerCharacterState* InPlayerState);
	
	
	void HandleAllPlayerisReady();
	bool IsAllPlayersReady();
	
	void StartNone();
	void StartLobby();
	void StartStage();
	void MoveToStage();
	void ForceReadyPlayers(bool bIsready);
	void StartPhaseTimer();
	
	bool CheckAllPlayerStatePlace(EPlayerPlace InPlayerPlace);
	// every event that changes who is in the stage (escape, death, timeout, logout) re-checks here
	void TryReturnToLobby();
	
	void ClearObjects();

#pragma region Rank
public:
	void StartRankUpdateTimer();
	void StopRankUpdateTimer();

	UFUNCTION()
	void CalculatePlayerRank();

private:
	FTimerHandle RankUpdateTimerHandle;
	UPROPERTY(EditDefaultsOnly, Category = "Rank")
	float RankUpdateInterval = 0.1f;
#pragma endregion
	
private:
	UPROPERTY()
	int32 MaxInGamePlayerCount = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 LobbyDuration = 30;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 PlayDuration = 180;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	float ForceReadyRemainTime = 5.f;
	UPROPERTY()
	FTimerHandle PhaseCheckTimerHandle;
	UFUNCTION()
	void CheckRemainTime();
	uint8 bForced : 1;
	
	UPROPERTY()
	TArray<AActor*> LobbyPlayerStartPoints;
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, int32> LobbyPlayerStartPointMap;
	UPROPERTY()
	TArray<AActor*> StageSpawnLocationPoints;
};
