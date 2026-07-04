

#pragma once

#include "CoreMinimal.h"
#include "ExtractionShooter14/Game/Struct/PlayerDatas.h"
#include "Game/Struct/InventoryItemDatas.h"
#include "GameFramework/PlayerState.h"
#include "PlayerCharacterState.generated.h"

DECLARE_MULTICAST_DELEGATE(FPlayerInfoChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerStageInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerStageGoldChanged, int32, OldGold, int32, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerReadyChanged, uint8, bIsReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerbIsDeadChanged, uint8, bIsDead);

UENUM(BlueprintType)
enum class EPlayerPlace : uint8
{
	None,
	Lobby,
	Stage
};

UCLASS()
class EXTRACTIONSHOOTER14_API APlayerCharacterState : public APlayerState
{
	GENERATED_BODY()
public:
	
#pragma region Cheat
       	UFUNCTION(Server, Reliable)
       	void Server_SetGold(int32 SetGold);
		UFUNCTION(Server, Reliable)
       	void Server_SetStageGold(int32 SetGold);
#pragma endregion 
	
	APlayerCharacterState();
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData);
	const FPlayerDatas& GetPlayerInfo() const;
	
	void ClearPlayerStageInventory();
	void ApplyPlayerStageInventoryToPlayerStorage();
	
#pragma region Delegate
	FPlayerInfoChanged OnPlayerDataChanged;
	UPROPERTY(BlueprintAssignable)
	FPlayerStageInventoryChanged OnPlayerStageInventoryChanged;
	UPROPERTY(BlueprintAssignable)
	FPlayerStageGoldChanged OnPlayerStageGoldChanged;
	FPlayerReadyChanged OnPlayerReadyChanged;
	FPlayerbIsDeadChanged OnPlayerbIsDeadChanged;
#pragma endregion 
	
#pragma region OnRep_Function
	UFUNCTION()
	void OnRep_PlayerInfo();
	UFUNCTION()
	void OnRep_PlayerStageInventory();
	UFUNCTION()
	void OnRep_PlayerStageGold(int32 OldGold);
	UFUNCTION()
	void OnRep_PlayerStagePlace();
	UFUNCTION()
	void OnRep_bIsReady();
	UFUNCTION()
	void OnRep_bIsDead();
#pragma endregion 
	
#pragma region SetterGetter
	bool GetbIsDead() const { return bIsDead; }
	void SetbIsDead(bool bIsDeadd);
	
	UFUNCTION(BlueprintCallable)
	EPlayerPlace GetPlayerStagePlace();
	void SetPlayerStagePlace(EPlayerPlace InPlayerStagePlace);
	
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerReady(bool bReady);
	bool IsPlayerReady() const { return bIsReady; }
	
	void SetPlayerReady(bool SetReady);
	
	void AddPlayerStageGold(int32 AddGoldValue);
#pragma endregion 
	
#pragma region Properties
protected:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FPlayerDatas PlayerInfo;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStageInventory)
	TArray<FInventoryItemData> PlayerStageInventory;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStageGold)
	int32 PlayerStageGold;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStagePlace)
	EPlayerPlace PlayerPlace;
	
	UPROPERTY(ReplicatedUsing=OnRep_bIsReady)
	uint8 bIsReady : 1;
	
	UPROPERTY(ReplicatedUsing=OnRep_bIsDead)
	uint8 bIsDead : 1;
#pragma endregion 
	
// =========================================================== MainPlayerCharacterState

public:

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void InitiateStats();
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
