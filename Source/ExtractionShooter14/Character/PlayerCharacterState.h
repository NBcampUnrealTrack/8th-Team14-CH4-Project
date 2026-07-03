

#pragma once

#include "CoreMinimal.h"
#include "ExtractionShooter14/Game/Struct/PlayerDatas.h"
#include "GameFramework/PlayerState.h"
#include "PlayerCharacterState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerInfoChanged, FPlayerDatas, PlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerReadyChanged, uint8, bIsReady);

UCLASS()
class EXTRACTIONSHOOTER14_API APlayerCharacterState : public APlayerState
{
	GENERATED_BODY()
public:
	APlayerCharacterState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData);
	
	const FPlayerDatas& GetPlayerInfo() const;
	
	FPlayerInfoChanged OnPlayerDataChanged;
	FPlayerReadyChanged OnPlayerReadyChanged;
	
	UFUNCTION()
	void OnRep_PlayerInfo();
	UFUNCTION()
	void OnRep_bIsReady();
	
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerReady(bool bReady);
	bool IsPlayerReady() const { return bIsReady; }
	
	void SetPlayerReady(bool SetReady);
#pragma region Cheat
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void PlusPlayerCoin();
#pragma endregion 
protected:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FPlayerDatas PlayerInfo;
	
	UPROPERTY(ReplicatedUsing=OnRep_bIsReady)
	uint8 bIsReady : 1;
	
// =========================================================== MainPlayerCharacterState

public:

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetMaxHealth() const { return MaxHealth; }
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Player|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Health")
	float MaxHealth = 100.0f;

protected:

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void ApplyDamage(float DamageAmount);
};
