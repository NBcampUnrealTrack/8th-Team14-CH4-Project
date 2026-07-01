#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TestGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChanedPlayerInGameCount, int32, InNewPlayerCount);

UCLASS()
class EXTRACTIONSHOOTER14_API ATestGameState : public AGameStateBase
{
	GENERATED_BODY()
public:	
	FChanedPlayerInGameCount OnChangedPlayerInGameCount;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetPlayerInGameCount_GS(int32 InNewPlayerInGameCount);
	int32 GetPlayerInGameCount_GS() const;
private:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInGameCount_GS)
	int32 PlayerInGameCount_GS;
	UFUNCTION()
	void OnRep_PlayerInGameCount_GS();
};
