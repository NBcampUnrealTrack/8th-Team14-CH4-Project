#include "PlayerCharacterState.h"

#include "MyPlayerController.h"
#include "TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"
#include "TestGameState.h"

void APlayerCharacterState::Server_SetStageItem_Implementation(const FInventoryItemData InItemName)
{
	AddItemInPlayerStageInventory(InItemName);
}

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
	,bIsDead(false)
{
	
}

void APlayerCharacterState::InitializeInventoryMaxSlot(int32 SlotCount)
{
	PlayerStageInventory.Init(FInventoryItemData(), SlotCount);
	PlayerInfo.ItemStorage.Init(FInventoryItemData(), SlotCount);
}

void APlayerCharacterState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInfo);
	DOREPLIFETIME(ThisClass, PlayerCommitCombatInfo);
	DOREPLIFETIME(ThisClass, bIsParticipatedInThisRound);
	DOREPLIFETIME(ThisClass, PlayerPlace);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerLobbyIndex, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerStageInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerStageGold, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, bIsReady);
	DOREPLIFETIME(ThisClass, bIsDead);
	DOREPLIFETIME(ThisClass, CurrentHealth);
	DOREPLIFETIME(ThisClass, Coin);
}

void APlayerCharacterState::ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData)
{
	// only initiate in server
	if (!HasAuthority()) return;
	
	PlayerInfo = ApplyPlayerData;
	PlayerStageInventory = ApplyPlayerData.ItemStorage;
	
	OnPlayerStageInventoryChanged.Broadcast();
	OnPlayerInfoChanged.Broadcast();
}

FPlayerDatas& APlayerCharacterState::GetPlayerInfo()
{
	return PlayerInfo;
}

void APlayerCharacterState::BuyItem_Implementation(const FInventoryItemData& InItemData, int32 InItemCost)
{
	if (HasAuthority())
	{
		AMyPlayerController* PC = Cast<AMyPlayerController>(GetPlayerController());
		if (!IsValid(PC)) return;

		FPlayerDatas& Info = GetPlayerInfo();

		// not enough money
		if (Info.GoldStorage < InItemCost)
		{
			PC->OnReceiveBuyItemResult.Broadcast(InItemData.ItemName, false, INDEX_NONE);
			PC->Client_OnReceiveBuyItemResult(InItemData.ItemName, false, INDEX_NONE);
			return;
		}

		// yes money try put in
		int32 AddedSlotIndex = AddItemInPlayerStageInventory(InItemData);
		// put > return index // no > return null
		if (AddedSlotIndex == INDEX_NONE)
		{
			// full inventory
			PC->OnReceiveBuyItemResult.Broadcast(InItemData.ItemName, false, INDEX_NONE);
			PC->Client_OnReceiveBuyItemResult(InItemData.ItemName, false, INDEX_NONE);
			return;
		}

		// yes money, have empty slot, not full
		Info.GoldStorage -= InItemCost;
		
		PC->OnReceiveBuyItemResult.Broadcast(InItemData.ItemName, true, AddedSlotIndex);
		PC->Client_OnReceiveBuyItemResult(InItemData.ItemName, true, AddedSlotIndex);
	}
}

void APlayerCharacterState::SetLobbyIndex(int32 PlayerSlotIndex)
{
	PlayerLobbyIndex = PlayerSlotIndex;
}

const int32 APlayerCharacterState::GetLobbyIndex() const
{
	return PlayerLobbyIndex;
}

void APlayerCharacterState::SetIsParticipatedInThisRound(bool InIsParticipatedInThisRound)
{
	if (HasAuthority())
	{
		bIsParticipatedInThisRound = InIsParticipatedInThisRound;
	}
}

bool APlayerCharacterState::GetIsParticipatedInThisRound() const
{
	return bIsParticipatedInThisRound;
}

void APlayerCharacterState::ClearPlayerStageInventoryGold()
{
	if (HasAuthority())
	{
		//PlayerStageInventory.Reset();
		PlayerStageGold = 0;
	}
}

void APlayerCharacterState::ClearPlayerStageInventory()
{
	if (HasAuthority())
	{
		for (FInventoryItemData& Item : PlayerStageInventory)
		{
			Item = FInventoryItemData();  // ItemName=NAME_None, ItemCount=0
		}

		//PlayerStageGold = 0;
		OnPlayerStageInventoryChanged.Broadcast();  // UI 갱신
	}
}

void APlayerCharacterState::ClearPlayerinfoInventory()
{
	if (HasAuthority())
	{
		for (FInventoryItemData& Item : PlayerInfo.ItemStorage)
		{
			Item = FInventoryItemData();  // ItemName=NAME_None, ItemCount=0
		}

		//PlayerStageGold = 0;
	}
}

void APlayerCharacterState::ApplyPlayerStageInventoryToPlayerStorage()
{
	// when playerstorage = add stack = chang go = container 
#pragma region When use PlayerStorage > Container
	// if (HasAuthority())
	// {
	// 	bool bIsItemChanged = false;
	// 	
	// 	for (const FInventoryItemData& StageItem : PlayerStageInventory)
	// 	{
	// 		if (StageItem.ItemName == NAME_None) // no item slot
	// 		{
	// 			continue;
	// 		}
	//
	// 		FInventoryItemData* FoundItem1 = PlayerInfo.ItemStorage.FindByPredicate(
	// 			[&StageItem](const FInventoryItemData& StorageItem)
	// 			{
	// 				return StorageItem.ItemName == StageItem.ItemName;
	// 			});
	//
	// 		if (FoundItem1)
	// 		{
	// 			FoundItem1->ItemCount += StageItem.ItemCount;
	// 		}
	// 		else // new item?
	// 		{
	// 			FInventoryItemData* FoundItem2 = PlayerInfo.ItemStorage.FindByPredicate(
	// 				[](const FInventoryItemData& StorageItem)
	// 				{
	// 					return StorageItem.ItemName == NAME_None;
	// 				});
	// 			if (FoundItem2)
	// 			{ // empty slot exist
	// 				*FoundItem2 = StageItem;
	// 			}
	// 			else
	// 			{
	// 				// full inventory
	// 				continue;
	// 			}
	// 		}
	// 		bIsItemChanged = true;
	// 	}
	//
	// 	if (PlayerStageGold > 0)
	// 	{
	// 		PlayerInfo.GoldStorage += PlayerStageGold;
	// 		bIsItemChanged = true;
	// 	}
	//
	// 	if (bIsItemChanged)
	// 	{
	// 		OnPlayerInfoChanged.Broadcast();
	// 	}
	// }
#pragma endregion 
	
	if (!HasAuthority()) return;

	// stageinven = containerinven
	PlayerInfo.ItemStorage = PlayerStageInventory;

	// add gold
	if (PlayerStageGold > 0)
	{
		PlayerInfo.GoldStorage += PlayerStageGold;
		PlayerStageGold = 0;   // stage gold clear
	}

	OnPlayerInfoChanged.Broadcast();
	
}

void APlayerCharacterState::Server_SetPlayerReady_Implementation(bool bReady)
{
	// in server
	SetPlayerReady(bReady);
	
	ATestGameMode* GameModeBaseES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeBaseES)) return;
	
	GameModeBaseES->HandleAllPlayerisReady();
	
}

void APlayerCharacterState::TakenDamage(
	float ActualDamage,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{// is in server
	if (!HasAuthority()) return;
	float OldValue = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);
	
	// ## todo::record combatinfo ##
	UE_LOG(LogTemp, Warning, TEXT("SERVER :: %.1f / %.1f"),CurrentHealth,MaxHealth);
	
	AddPlayerCombatInfo_DamageTaken(ActualDamage);
	APlayerCharacterState* AttackerPlayerState = EventInstigator->GetPlayerState<APlayerCharacterState>();
	
	if (IsValid(AttackerPlayerState))
	{
		AttackerPlayerState->AddPlayerCombatInfo_DamageDealt(ActualDamage);
	}
	
	if (CurrentHealth <= 0.0f && bIsDead == false)
	{
		UE_LOG(LogTemp, Error, TEXT("[DEAD] %s bIsDead=%d Role=%d Frame=%llu"),
			*GetPlayerName(), bIsDead ? 1 : 0,
			(int32)GetLocalRole(), (uint64)GFrameCounter);

		SetbIsDead(true);
		if (IsValid(AttackerPlayerState))
		{
			AttackerPlayerState->AddPlayerCombatInfo_KillCount(1);
		}

		// final result for death ResultUI (survive time stops here)
		UpdateSurviveTime();
		CommitCombatInfo();
	}
	
	OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
	//OnPlayerCombatInfoChanged.Broadcast();
	ForceNetUpdate();
}
void APlayerCharacterState::InitiateStats()
{
	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		//
		SetbIsDead(false);
		SetPlayerReady(false);
	}
}

void APlayerCharacterState::Heal(float HealAmount)
{
	if (!HasAuthority())
	{
		return;
	}
	
	float OldValue = CurrentHealth;
	
	CurrentHealth = FMath::Clamp(
		CurrentHealth + HealAmount,
		0.f,
		MaxHealth
	);

	OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);

	ForceNetUpdate();
}

void APlayerCharacterState::OnRep_PlayerInfo()
{
	//todo:: ui, inventory update
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("[OnRep_PlayerInfo] This=%s Gold=%d LocalRole=%s"),
		*GetNameSafe(this),
		PlayerInfo.GoldStorage,
		
		
		*UEnum::GetValueAsString(GetLocalRole())
	);
	UE_LOG(
	LogTemp,
	Warning,
	TEXT("[OnRep_PlayerInfo] This=%s Gold=%d IsBound=%d"),
	*GetNameSafe(this),
	PlayerInfo.GoldStorage,
	OnPlayerInfoChanged.IsBound()
);
	OnPlayerInfoChanged.Broadcast();
}

void APlayerCharacterState::OnRep_PlayerCommitCombatInfo(const FCollectCombatInfo& OldCombatInfo)
{
	//OnPlayerCombatInfoChanged.Broadcast();
	//if (OldCombatInfo.Rank == PlayerCombatInfo.Rank) return;

	OnPlayerCombatInfoChanged.Broadcast();
	// 	OnPlayerRankChanged.Broadcast();
}

void APlayerCharacterState::OnRep_PlayerStageInventory()
{
	OnPlayerStageInventoryChanged.Broadcast();
	UKismetSystemLibrary::PrintString(GetWorld(), FString(TEXT("CLIENT :: StageInventoryChanged")));
}

void APlayerCharacterState::OnRep_PlayerStageGold(int32 OldGold)
{
	UE_LOG(LogTemp, Warning, TEXT("CLIENT OnRep_PlayerStageGold Old=%d New=%d"), OldGold, PlayerStageGold);

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
}

void APlayerCharacterState::OnRep_PlayerLobbyIndex()
{
	OnPlayerLobbyIndexChanged.Broadcast();
}

void APlayerCharacterState::OnRep_Health(float OldValue)
{
	OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("CLIENT :: %.1f / %.1f"),CurrentHealth,MaxHealth);
}

void APlayerCharacterState::OnRep_Coin()
{
}

void APlayerCharacterState::OnRep_bIsReady()
{
	OnPlayerReadyChanged.Broadcast(bIsReady);
}

void APlayerCharacterState::OnRep_bIsDead()
{
	UE_LOG(LogTemp, Error, TEXT("[DEAD] %s bIsDead=%d Role=%d Frame=%llu"),
		*GetPlayerName(), bIsDead ? 1 : 0,
		(int32)GetLocalRole(), (uint64)GFrameCounter);
	OnPlayerbIsDeadChanged.Broadcast(this, bIsDead);
}

void APlayerCharacterState::OnRep_PlayerStagePlace()
{
	
}

void APlayerCharacterState::SetPlayerReady(bool SetReady)
{
	if (bIsReady == SetReady)
		return;

	bIsReady = SetReady;

	OnRep_bIsReady();
	ForceNetUpdate();
}

void APlayerCharacterState::AddPlayerStageGold(int32 AddGoldValue)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 OldGold = PlayerStageGold;

	PlayerStageGold += AddGoldValue;

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	ForceNetUpdate();
}

int32 APlayerCharacterState::AddItemInPlayerStageInventory(const FInventoryItemData& InItemName)
{
	if (HasAuthority())
	{
		int32 ChangedSlotIndex = INDEX_NONE;

		int32 FoundIndex = PlayerStageInventory.IndexOfByPredicate(
			[&InItemName](const FInventoryItemData& StorageItem)
			{
				return StorageItem.ItemName == InItemName.ItemName
					&& StorageItem.ItemName != NAME_None; // itemName = None : defence code
			});

		if (FoundIndex != INDEX_NONE) // find index
		{
			PlayerStageInventory[FoundIndex].ItemCount += InItemName.ItemCount;
			ChangedSlotIndex = FoundIndex;
		}
		else // not find index
		{
			int32 EmptyIndex = PlayerStageInventory.IndexOfByPredicate(
				[](const FInventoryItemData& StorageItem)
				{
					return StorageItem.ItemName == NAME_None; // find empty array slot
				});

			if (EmptyIndex == INDEX_NONE) // not find empty slot
			{
				return INDEX_NONE;  // full
			}
					// not full find empty array slot
			PlayerStageInventory[EmptyIndex] = InItemName;
			ChangedSlotIndex = EmptyIndex;
		}
		
		PlayerInfo.ItemStorage = PlayerStageInventory; // when buy item. save item to save storage
		OnPlayerStageInventoryChanged.Broadcast();
		ForceNetUpdate();

		return ChangedSlotIndex;   // cached index when [+itemcount, +item]
	}

	return INDEX_NONE; // not authority
}

const TArray<FInventoryItemData>& APlayerCharacterState::GetPlayerStageInventory() const
{
	return PlayerStageInventory;
}

void APlayerCharacterState::AddPlayerCombatInfo_DamageDealt(const float InGiveDmg)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.DamageDealt += InGiveDmg;

	}
}

void APlayerCharacterState::AddPlayerCombatInfo_DamageTaken(const float InReceiveDmg)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.DamageTaken += InReceiveDmg;

	}
}

void APlayerCharacterState::AddPlayerCombatInfo_KillCount(const int32 InKillCount)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.KillCount += InKillCount;
		
	}
}

void APlayerCharacterState::AddPlayerCombatInfo_LootCount(const int32 InLootCount)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.LootCount += InLootCount;
		
	}
}

bool APlayerCharacterState::SetPlayerCombatInfo_Rank(const int32 InRank)
{
	if (!HasAuthority()) return false;

	PlayerCombatInfo.Rank = InRank;

	// same rank = no commit = no replicate
	if (PlayerCommitCombatInfo.Rank == InRank) return false;

	CommitCombatInfo();
	return true;
}

void APlayerCharacterState::CommitCombatInfo()
{
	if (!HasAuthority()) return;

	PlayerCommitCombatInfo = PlayerCombatInfo; // rep-struct = nonrep-struct
	OnPlayerCombatInfoChanged.Broadcast();
	ForceNetUpdate();
}

const FCollectCombatInfo& APlayerCharacterState::GetPlayerCombatInfo() const
{
	return PlayerCommitCombatInfo;
}

const FCollectCombatInfo& APlayerCharacterState::GetPlayerWorkingCombatInfo() const
{
	return PlayerCombatInfo;
}

void APlayerCharacterState::UpdateSurviveTime()
{
	if (HasAuthority())
	{
		ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>();
		if (!IsValid(GS)) return;

		if (PlayerCombatInfo.StageStartServerTime <= 0.f) return;
		
		PlayerCombatInfo.SurviveTime =
			GS->GetServerWorldTimeSeconds() - PlayerCombatInfo.StageStartServerTime;

		CalCulateCombatScore();
	}
}

void APlayerCharacterState::StartSurviveTime()
{
	if (!HasAuthority()) return;

	ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>();
	if (!IsValid(GS)) return;

	PlayerCombatInfo.StageStartServerTime = GS->GetServerWorldTimeSeconds();
	PlayerCombatInfo.SurviveTime = 0.f;

	CalCulateCombatScore();
}

void APlayerCharacterState::CalCulateCombatScore()
{
	PlayerCombatInfo.Score =
	FMath::RoundToInt(PlayerCombatInfo.DamageDealt * 2.f) -
	FMath::RoundToInt(PlayerCombatInfo.DamageTaken * 1.f) +
	(PlayerCombatInfo.KillCount * 5) +
	(PlayerCombatInfo.LootCount * 30) +
	FMath::RoundToInt(PlayerCombatInfo.SurviveTime * 1.5f);
}

void APlayerCharacterState::InitializeCombatScore()
{
	PlayerCombatInfo = FCollectCombatInfo();
	PlayerCommitCombatInfo = PlayerCombatInfo; // clear last round result
}

void APlayerCharacterState::SetbIsDead(bool bIsDeadd)
{
	if (bIsDead == bIsDeadd)
		return;

	bIsDead = bIsDeadd;

	OnRep_bIsDead(); // 서버 즉시 처리
	ForceNetUpdate();
}

EPlayerPlace APlayerCharacterState::GetPlayerStagePlace()
{
	return PlayerPlace;
}

void APlayerCharacterState::SetPlayerStagePlace(EPlayerPlace InPlayerStagePlace)
{
	if (PlayerPlace == InPlayerStagePlace)	return;
	
	PlayerPlace = InPlayerStagePlace;
	OnRep_PlayerStagePlace();
	ForceNetUpdate();
}

void APlayerCharacterState::SetPlayerStageGold(int32 NewGold)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 OldGold = PlayerStageGold;

	PlayerStageGold = FMath::Max(0, NewGold);

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);

	ForceNetUpdate();
}

#pragma region Cheat
	
void APlayerCharacterState::Server_SetGold_Implementation(int32 SetGold)
{
	PlayerInfo.GoldStorage = SetGold;
}

//서버 즉시갱신 이벤트 - 성윤
void APlayerCharacterState::Server_SetStageGold_Implementation(int32 SetGold)
{
	const int32 OldGold = PlayerStageGold;

	PlayerStageGold += SetGold;

	UE_LOG(LogTemp, Warning, TEXT("SERVER StageGold Old=%d New=%d"), OldGold, PlayerStageGold);

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	ForceNetUpdate();
}

//힐팩갯수 줄이기 - 성윤
bool APlayerCharacterState::ConsumeItemInPlayerStageInventory(FName ItemName, int32 ConsumeCount)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (ItemName.IsNone() || ConsumeCount <= 0)
	{
		return false;
	}

	const int32 FoundIndex = PlayerStageInventory.IndexOfByPredicate(
		[ItemName](const FInventoryItemData& StorageItem)
		{
			return StorageItem.ItemName == ItemName
				&& StorageItem.ItemName != NAME_None;
		}
	);

	if (FoundIndex == INDEX_NONE)
	{
		return false;
	}

	FInventoryItemData& FoundItem = PlayerStageInventory[FoundIndex];

	if (FoundItem.ItemCount < ConsumeCount)
	{
		return false;
	}

	FoundItem.ItemCount -= ConsumeCount;

	if (FoundItem.ItemCount <= 0)
	{
		FoundItem = FInventoryItemData();
	}

	OnPlayerStageInventoryChanged.Broadcast();
	ForceNetUpdate();

	return true;
}
#pragma endregion 
