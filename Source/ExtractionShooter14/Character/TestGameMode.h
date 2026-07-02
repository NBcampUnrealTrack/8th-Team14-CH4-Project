#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestGameMode.generated.h"

class AMyPlayerController;

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	ATestGameMode();
	virtual void BeginPlay() override;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	int32 GetAvailableStartIndex() const;
	// virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	FTransform GetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void SetPlayerLobbyStartPoint(AMyPlayerController* TargetController);
	void RespawnPlayerAtLobbyStartPoint(AMyPlayerController* TargetController);
	
	int32 GetInGamePlayerCount() const;
	void SetInGamePlayerCount(int32 InNewPlayerCount);
	int32 GetMaxInGamePlayerCount() const;
	void SetMaxInGamePlayerCount(int32 InNewMaxPlayerCount);
	
	void HandleAllPlayerisReady();
	bool IsAllPlayersReady();
private:
	UPROPERTY()
	int32 InGamePlayerCount = 0;
	UPROPERTY()
	int32 MaxInGamePlayerCount = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Timer", meta = (AllowPrivateAccess))
	int32 WaitingTime = 15;
	int32 RemainingTime;
	
	
	UPROPERTY()
	TArray<AActor*> LobbyPlayerStartPoints;
	UPROPERTY()
	TMap<TWeakObjectPtr<APlayerController>, int32> LobbyPlayerStartPointMap;
	UPROPERTY()
	TArray<AActor*> StageSpawnLocationActors;
};
