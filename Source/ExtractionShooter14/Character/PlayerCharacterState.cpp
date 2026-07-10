#include "PlayerCharacterState.h"
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
	PlayerStageInventory.Init(FInventoryItemData(), 12);
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
}

void APlayerCharacterState::ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData)
{
	// only initiate in server
	if (!HasAuthority()) return;
	
	PlayerInfo = ApplyPlayerData;
	OnPlayerInfoChanged.Broadcast();
}

FPlayerDatas& APlayerCharacterState::GetPlayerInfo()
{
	return PlayerInfo;
}

void APlayerCharacterState::BuyItem_Implementation(const FInventoryItemData& InItemData)
{
	if (HasAuthority())
	{
		FPlayerDatas& info = GetPlayerInfo();
		info.GoldStorage -= 100;
		
		AddItemInPlayerStageInventory(InItemData);
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

void APlayerCharacterState::ClearPlayerStageInventory()
{
	if (HasAuthority())
	{
		//PlayerStageInventory.Reset();
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
		OnPlayerInfoChanged.Broadcast();
		

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

void APlayerCharacterState::TakenDamage(
	float ActualDamage,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser
)
{// is in server
	if (!HasAuthority()) return;

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
		SetbIsDead(true);
		if (IsValid(AttackerPlayerState))
		{
			AttackerPlayerState->AddPlayerCombatInfo_KillCount(1);
		}
	}
	
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
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

void APlayerCharacterState::OnRep_Health()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	UE_LOG(LogTemp, Warning, TEXT("CLIENT :: %.1f / %.1f"),CurrentHealth,MaxHealth);
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
	if (!HasAuthority())
	{
		return;
	}

	const int32 OldGold = PlayerStageGold;

	PlayerStageGold += AddGoldValue;

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	ForceNetUpdate();
}

void APlayerCharacterState::AddItemInPlayerStageInventory(const FInventoryItemData& InItemName)
{
	if (HasAuthority())
	{
		bool bIsItemChanged = false;
		
		// 1. 같은 아이템 이름 찾기
		FInventoryItemData* FoundItem = PlayerStageInventory.FindByPredicate(
			[&InItemName](const FInventoryItemData& StorageItem)
			{
				return StorageItem.ItemName == InItemName.ItemName
					&& StorageItem.ItemName != NAME_None;
			}
		);

		if (FoundItem)
		{
			FoundItem->ItemCount += InItemName.ItemCount;
			bIsItemChanged = true;
		}
		else
		{
			// 2. 빈 슬롯 찾기
			FInventoryItemData* EmptySlot = PlayerStageInventory.FindByPredicate(
				[](const FInventoryItemData& StorageItem)
				{
					return StorageItem.ItemName == NAME_None;
				}
			);

			if (EmptySlot)
			{
				*EmptySlot = InItemName;
				bIsItemChanged = true;
			}
			else
			{
				// 인벤토리 가득 참
				return;
			}
		}

		if (bIsItemChanged)
		{
			OnPlayerStageInventoryChanged.Broadcast();
			ForceNetUpdate();
		}
	}
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
				
		//CalCulateCombatScore();
		
		// if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		// {
		// 	GS->MarkRank();
		// }
	}
}

void APlayerCharacterState::AddPlayerCombatInfo_DamageTaken(const float InReceiveDmg)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.DamageTaken += InReceiveDmg;
				
		//CalCulateCombatScore();
		
		// if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		// {
		// 	GS->MarkRank();
		// }
	}
}

void APlayerCharacterState::AddPlayerCombatInfo_KillCount(const int32 InKillCount)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.KillCount += InKillCount;
				
		//CalCulateCombatScore();
		
		// if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		// {
		// 	GS->MarkRank();
		// }
	}
}

void APlayerCharacterState::AddPlayerCombatInfo_LootCount(const int32 InLootCount)
{
	if (HasAuthority())
	{
		PlayerCombatInfo.LootCount += InLootCount;
		
		//CalCulateCombatScore();
		
		// if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		// {
		// 	GS->MarkRank();
		// }
	}
}

void APlayerCharacterState::SetPlayerCombatInfo_Rank(const int32 InRank)
{
	if (HasAuthority())
	{
		//if (PlayerCombatInfo.Rank == InRank) return;
		
		PlayerCombatInfo.Rank = InRank;
		
		
		//CalCulateCombatScore();
		
		// if (ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>())
		// {
		// 	GS->MarkRank();
		// }
		PlayerCommitCombatInfo = PlayerCombatInfo;
		
		OnPlayerCombatInfoChanged.Broadcast();
		//OnPlayerRankChanged.Broadcast();
	}
}

const FCollectCombatInfo& APlayerCharacterState::GetPlayerCombatInfo() const
{
	return PlayerCommitCombatInfo;
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

//서버 즉시갱신 이벤트 - 성윤
void APlayerCharacterState::Server_SetStageGold_Implementation(int32 SetGold)
{
	const int32 OldGold = PlayerStageGold;

	PlayerStageGold += SetGold;

	UE_LOG(LogTemp, Warning, TEXT("SERVER StageGold Old=%d New=%d"), OldGold, PlayerStageGold);

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	ForceNetUpdate();
}

#pragma endregion 