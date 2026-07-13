#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

struct FPhaseTimerData;
enum class EPlayerPlace : uint8;
class APlayerCharacterState;
class AMyPlayerController;

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ATestGameMode();
	virtual void BeginPlay() override;
	
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	int32 GetAvailableStartIndex() const;
	// virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	FTransform GetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void SetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController);
	void TimeOutReSetPlayer(AMyPlayerController* TargetController);
	void InitializePlayerStats(APlayerCharacterState* InPlayerState);
	
	int32 GetInGamePlayerCount() const;
	void SetInGamePlayerCount(int32 InNewPlayerCount);
	int32 GetMaxInGamePlayerCount() const;
	void SetMaxInGamePlayerCount(int32 InNewMaxPlayerCount);
	
	void HandleAllPlayerisReady();
	bool IsAllPlayersReady();
	
	void StartNone();
	void StartLobby();
	void StartStage();
	void MoveToStage();
	void ForceReadyPlayers(bool bIsready);
	void StartPhaseTimer();
	
	bool CheckAllPlayerStatePlace(EPlayerPlace InPlayerPlace);
private:
	UPROPERTY()
	int32 InGamePlayerCount = 0;
	UPROPERTY()
	int32 MaxInGamePlayerCount = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 LobbyDuration = 30;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 PlayDuration = 50;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 CountDownTime = 5;
	UPROPERTY()
	FTimerHandle CheckRemainLobbyTimeHandle;
	// UPROPERTY()
	// FTimerHandle CheckRemainStageTimeHandle;
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
