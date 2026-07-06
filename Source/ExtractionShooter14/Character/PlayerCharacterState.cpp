#include "PlayerCharacterState.h"
#include "TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
	,bIsDead(false)
{
	PlayerStageInventory.Reserve(30);
}

void APlayerCharacterState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInfo);
	//DOREPLIFETIME(ThisClass, PlayerCombatInfo);
	DOREPLIFETIME(ThisClass, PlayerPlace);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerLobbyIndex, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerStageInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerStageGold, COND_OwnerOnly);
	DOREPLIFETIME(ThisClass, bIsReady);
	DOREPLIFETIME(ThisClass, bIsDead);
	DOREPLIFETIME(ThisClass, CurrentHealth);
}

void APlayerCharacterState::ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData)
{
	// only initiate in server
	if (!HasAuthority()) return;
	
	PlayerInfo = ApplyPlayerData;
	OnPlayerDataChanged.Broadcast();
}

const FPlayerDatas& APlayerCharacterState::GetPlayerInfo() const
{
	return PlayerInfo;
}

void APlayerCharacterState::SetLobbyIndex(int32 PlayerSlotIndex)
{
	PlayerLobbyIndex = PlayerSlotIndex;
}

const int32 APlayerCharacterState::GetLobbyIndex() const
{
	return PlayerLobbyIndex;
}

void APlayerCharacterState::ClearPlayerStageInventory()
{
	if (HasAuthority())
	{
		PlayerStageInventory.Reset();
		PlayerStageGold = 0;
	}
}

void APlayerCharacterState::ApplyPlayerStageInventoryToPlayerStorage()
{
	if (HasAuthority())
	{
		bool bIsItemChanged = false;
		
		for (const FInventoryItemData& Items : PlayerStageInventory)
		{ // FoundItem have array's iterator
			FInventoryItemData* FoundItem =
			PlayerInfo.ItemStorage.FindByPredicate(
		[&Items](const FInventoryItemData& StorageItem)
				{
					return StorageItem.ItemName == Items.ItemName;
				});

			if (FoundItem)
			{
				// PlayerInfo.ItemStorage[].ItemCount += 10; SameMeaning
				// FoundItem = PlayerInfo.ItemStorage[] ,  return Array's iterator
				FoundItem->ItemCount += Items.ItemCount;
				bIsItemChanged=true;
			}
			else
			{
				// no same item
				// add total
				PlayerInfo.ItemStorage.Add(Items);
				bIsItemChanged=true;
			}
		
		}
		
		if (PlayerStageGold > 0)
		{
			PlayerInfo.GoldStorage += PlayerStageGold;
			bIsItemChanged=true;
			
		}
		
		if (bIsItemChanged) // if no add pass broadcast
		OnPlayerDataChanged.Broadcast();
		

	}
}

void APlayerCharacterState::Server_SetPlayerReady_Implementation(bool bReady)
{
	// in server
	SetPlayerReady(bReady);
	
	ATestGameMode* GameModeBaseES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeBaseES)) return;
	
	GameModeBaseES->HandleAllPlayerisReady();
	
}

void APlayerCharacterState::ApplyDamage(float DamageAmount)
{// is in server
	if (!HasAuthority()) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	
	if (CurrentHealth <= 0.0f && bIsDead == false)
	{
		//if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
		//{
			SetbIsDead(true);
			//OnRep_bIsDead();
			//PlayerCharacter->Die();
			
		//}
	}
}
void APlayerCharacterState::InitiateStats()
{
	if (HasAuthority())
	{
		CurrentHealth = MaxHealth;
		PlayerCombatInfo = FCollectCombatInfo();
		SetbIsDead(false);
		SetPlayerReady(false);
	}
}

void APlayerCharacterState::OnRep_PlayerInfo()
{
	//todo:: ui, inventory update
	UKismetSystemLibrary::PrintString
	(
		GetWorld(),
		FString::Printf(TEXT("Loaded PlayerData::Coin = %d"), PlayerInfo.GoldStorage),
		true,
		true,
		FColor::Red,
		5.f
		);
	
	OnPlayerDataChanged.Broadcast();
}

void APlayerCharacterState::OnRep_PlayerStageInventory()
{
	OnPlayerStageInventoryChanged.Broadcast();
}

void APlayerCharacterState::OnRep_PlayerStageGold(int32 OldGold)
{
	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
}

void APlayerCharacterState::OnRep_PlayerLobbyIndex()
{
	OnPlayerLobbyIndexChanged.Broadcast();
}

void APlayerCharacterState::OnRep_Health()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void APlayerCharacterState::OnRep_bIsReady()
{
	OnPlayerReadyChanged.Broadcast(bIsReady);
}

void APlayerCharacterState::OnRep_bIsDead()
{
	OnPlayerbIsDeadChanged.Broadcast(bIsDead);
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
	if (HasAuthority())
	{
		const int32 OldGold = PlayerStageGold;

		PlayerStageGold += AddGoldValue;

		OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	}
}

void APlayerCharacterState::AddItemInPlayerStageInventory(const FInventoryItemData& InItemName)
{
	if (HasAuthority())
	{
		bool bIsItemChanged = false;
		
		FInventoryItemData* FoundItem =
	PlayerStageInventory.FindByPredicate(
	[&InItemName](const FInventoryItemData& StorageItem)
	{
		return StorageItem.ItemName == InItemName.ItemName;
	});

		if (FoundItem)
		{
			// PlayerInfo.ItemStorage[].ItemCount += 10; SameMeaning
			// FoundItem = PlayerInfo.ItemStorage[] ,  return Array's iterator
			FoundItem->ItemCount += InItemName.ItemCount;
			bIsItemChanged=true;
		}
		else
		{
			// no same item
			// add total
			PlayerStageInventory.Add(InItemName);
			bIsItemChanged=true;
		}
		
	if (bIsItemChanged) // if no add pass broadcast
		OnPlayerStageInventoryChanged.Broadcast();
	}
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

#pragma region Cheat
	
void APlayerCharacterState::Server_SetGold_Implementation(int32 SetGold)
{
	PlayerInfo.GoldStorage = SetGold;
}

void APlayerCharacterState::Server_SetStageGold_Implementation(int32 SetGold)
{
	PlayerStageGold = SetGold;
}

#pragma endregion 