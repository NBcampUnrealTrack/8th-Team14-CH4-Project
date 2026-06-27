

#pragma once

#include "CoreMinimal.h"
#include "ExtractionShooter14/Game/Struct/PlayerDatas.h"
#include "GameFramework/PlayerState.h"
#include "PlayerCharacterState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerInfoChanged, FPlayerDatas, PlayerData);

UCLASS()
class EXTRACTIONSHOOTER14_API APlayerCharacterState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData);
	
	const FPlayerDatas& GetPlayerInfo() const;
	
	FPlayerInfoChanged OnPlayerDataChanged;
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PlusPlayerCoin();
protected:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FPlayerDatas PlayerInfo;
	UFUNCTION()
	void OnRep_PlayerInfo();
private:
};
