

#pragma once

#include "CoreMinimal.h"
#include "ExtractionShooter14/Game/Struct/PlayerDatas.h"
#include "Game/Struct/InventoryItemDatas.h"
#include "Game/Struct/CollectCombatInfo.h"
#include "GameFramework/PlayerState.h"
#include "PlayerCharacterState.generated.h"

DECLARE_MULTICAST_DELEGATE(FPlayerInfoChanged);
DECLARE_MULTICAST_DELEGATE(FPlayerLobbyIndexChanged);
DECLARE_MULTICAST_DELEGATE(FPlayerCombatInfoChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerStageInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerStageGoldChanged, int32, OldGold, int32, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerReadyChanged, uint8, bIsReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerbIsDeadChanged, APlayerCharacterState*, PlayerCharacterState, uint8, bIsDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnHealthChanged,
	float, OldValue,
	float, NewValue,
	float, MaxHealth
);

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
		UFUNCTION(Server, Reliable,BlueprintCallable)
       	void Server_SetStageGold(int32 SetGold);
		UFUNCTION(Server, Reliable)
		void Server_SetStageItem(const FInventoryItemData InItemName);
#pragma endregion 
	
	APlayerCharacterState();
	
	void InitializeInventoryMaxSlot(int32 SlotCount);
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	void ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData);
	
	FPlayerDatas& GetPlayerInfo();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void BuyItem(const FInventoryItemData& InItemData, int32 InItemcost);
	
	void ClearPlayerStageInventoryGold();
	void ClearPlayerStageInventory();
	void ClearPlayerinfoInventory();
	void ApplyPlayerStageInventoryToPlayerStorage();

	UPROPERTY(ReplicatedUsing = OnRep_Coin)
	int32 Coin = 0;
	
#pragma region Delegate
	FPlayerInfoChanged OnPlayerInfoChanged;
	FPlayerLobbyIndexChanged OnPlayerLobbyIndexChanged;
	FPlayerCombatInfoChanged OnPlayerCombatInfoChanged;
	UPROPERTY(BlueprintAssignable)
	FPlayerStageInventoryChanged OnPlayerStageInventoryChanged;
	UPROPERTY(BlueprintAssignable)
	FPlayerStageGoldChanged OnPlayerStageGoldChanged;
	UPROPERTY(BlueprintAssignable, Category = "Player|Health")
	FOnHealthChanged OnHealthChanged;
	FPlayerReadyChanged OnPlayerReadyChanged;
	FPlayerbIsDeadChanged OnPlayerbIsDeadChanged;
#pragma endregion 
	
#pragma region OnRep_Function
	UFUNCTION()
	void OnRep_PlayerInfo();
	UFUNCTION()
	void OnRep_PlayerCommitCombatInfo(const FCollectCombatInfo& OldCombatInfo);
	UFUNCTION()
	void OnRep_PlayerStageInventory();
	UFUNCTION()
	void OnRep_PlayerStageGold(int32 OldGold);
	UFUNCTION()
	void OnRep_PlayerLobbyIndex();
	UFUNCTION()
	void OnRep_PlayerStagePlace();
	UFUNCTION()
	void OnRep_bIsReady();
	UFUNCTION()
	void OnRep_bIsDead();
	UFUNCTION()
	void OnRep_Health(float OldValue);
	UFUNCTION()
	void OnRep_Coin();
#pragma endregion 
	
#pragma region SetterGetter(LobbyFunction)
	bool GetbIsDead() const { return bIsDead; }
	void SetbIsDead(bool bIsDeadd);
	
	UFUNCTION(BlueprintCallable)
	EPlayerPlace GetPlayerStagePlace();
	void SetPlayerStagePlace(EPlayerPlace InPlayerStagePlace);

	
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerReady(bool bReady);
	bool IsPlayerReady() const { return bIsReady; }
	
	void SetPlayerReady(bool SetReady);
	
	void SetLobbyIndex(int32 PlayerSlotIndex);
	const int32 GetLobbyIndex() const;
	
	void SetIsParticipatedInThisRound(bool InIsParticipatedInThisRound);
	bool GetIsParticipatedInThisRound() const;
#pragma endregion 
	
#pragma region ItemFunction
	
	UFUNCTION()  // Put Gold In StageGold
	void AddPlayerStageGold(int32 AddGoldValue); //server only
	UFUNCTION(BlueprintCallable) // Put Item In StageInventory
	int32 AddItemInPlayerStageInventory(const FInventoryItemData& InItemName); //serveronly
	// FInventoryItemData == <FName, int32>
	
	UFUNCTION(BlueprintCallable)
	const TArray<FInventoryItemData>& GetPlayerStageInventory() const;
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool ConsumeItemInPlayerStageInventory(FName ItemName, int32 ConsumeCount = 1);
#pragma endregion 
	
#pragma region CombatFunction
	
	void CalCulateCombatScore();
	void InitializeCombatScore();

	UFUNCTION()
	void AddPlayerCombatInfo_DamageDealt(const float InGiveDmg);
	UFUNCTION()
	void AddPlayerCombatInfo_DamageTaken(const float InReceiveDmg);
	UFUNCTION()
	void AddPlayerCombatInfo_KillCount(const int32 InKillCount);
	UFUNCTION()
	void AddPlayerCombatInfo_LootCount(const int32 InLootCount);
	UFUNCTION()
	bool SetPlayerCombatInfo_Rank(const int32 InRank); // return true = rank changed > committed(replicate)
	void CommitCombatInfo(); // working struct > replicated struct

	UFUNCTION(BlueprintCallable, Category = "Player|StageGold")
	int32 GetPlayerStageGold() const { return PlayerStageGold; }
	
	const FCollectCombatInfo& GetPlayerCombatInfo() const;
	const FCollectCombatInfo& GetPlayerWorkingCombatInfo() const; // server only, latest score
	void UpdateSurviveTime();
	void StartSurviveTime();
#pragma endregion 
	
#pragma region Properties
protected:
	UPROPERTY(ReplicatedUsing=OnRep_PlayerInfo)
	FPlayerDatas PlayerInfo;
	
	UPROPERTY()
	FCollectCombatInfo PlayerCombatInfo;
	UPROPERTY(ReplicatedUsing=OnRep_PlayerCommitCombatInfo)
	FCollectCombatInfo PlayerCommitCombatInfo;
	
	UPROPERTY(Replicated)
	bool bIsParticipatedInThisRound = false;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStageInventory)
	TArray<FInventoryItemData> PlayerStageInventory;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStageGold)
	int32 PlayerStageGold = 0;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerLobbyIndex)
	int32 PlayerLobbyIndex = INDEX_NONE;
	
	UPROPERTY(ReplicatedUsing=OnRep_PlayerStagePlace)
	EPlayerPlace PlayerPlace = EPlayerPlace::None;
	
	UPROPERTY(ReplicatedUsing=OnRep_bIsReady)
	uint8 bIsReady : 1;
	
	UPROPERTY(ReplicatedUsing=OnRep_bIsDead)
	uint8 bIsDead : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Player|Health")
    float CurrentHealth = 100.0f;
  
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player|Health")
    float MaxHealth = 100.0f;

#pragma endregion 
	
// =========================================================== MainPlayerCharacterState

public:
	
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void TakenDamage(
	float ActualDamage,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
);
	
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	float GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void InitiateStats();

	UFUNCTION(BlueprintCallable, Category = "Player|Health")
	void Heal(float HealAmount);

};
