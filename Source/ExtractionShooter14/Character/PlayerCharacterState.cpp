#include "PlayerCharacterState.h"

#include "MyPlayerController.h"
#include "TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"
#include "TestGameState.h"
#include "Game/DataManagerSubsystem.h"

// cheat
void APlayerCharacterState::Server_SetStageItem_Implementation(const FInventoryItemData InItemName)
{
#if UE_BUILD_SHIPPING
	return;
#endif
	AddItemInPlayerStageInventory(InItemName);
}

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
	,bIsDead(false)
{
	// rank changes are batched by update frequency instead of ForceNetUpdate
	SetNetUpdateFrequency(4.f);
	
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
	DOREPLIFETIME_CONDITION(ThisClass, MyRank, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, PlayerCombatInfo, COND_OwnerOnly);
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

void APlayerCharacterState::BuyItem_Implementation(FName ItemName, int32 Quantity)
{
	if (!HasAuthority()) return;

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetPlayerController());
	if (!IsValid(PC)) return;

	// result goes only through the client RPC (listen host runs it locally : one broadcast)
	auto SendResult = [PC, ItemName](bool bSuccess, int32 SlotIndex)
	{
		PC->Client_OnReceiveBuyItemResult(ItemName, bSuccess, SlotIndex);
	};

	// 1. validate the requested item and quantity
	if (ItemName.IsNone() || Quantity <= 0) return SendResult(false, INDEX_NONE);

	// 2. shop is only for lobby (RPC can be sent anytime, UI is not security)
	if (PlayerPlace != EPlayerPlace::Lobby) return SendResult(false, INDEX_NONE);

	// 3. the server chooses the unit price; reject overflow before multiplying int32 values
	const UDataManagerSubsystem* DataManager = GetGameInstance()->GetSubsystem<UDataManagerSubsystem>();
	const TOptional<int32> Price = DataManager ? DataManager->FindPrice(ItemName) : TOptional<int32>();
	if (!Price.IsSet() || Price.GetValue() < 0) return SendResult(false, INDEX_NONE);

	const int32 UnitPrice = Price.GetValue();
	if (UnitPrice > MAX_int32 / Quantity) return SendResult(false, INDEX_NONE);

	const int32 TotalPrice = UnitPrice * Quantity;
	FPlayerDatas& Info = GetPlayerInfo();
	if (TotalPrice > Info.GoldStorage) return SendResult(false, INDEX_NONE);

	// 4. add the complete quantity, or reject without charging (including stack overflow)
	FInventoryItemData PurchasedItem;
	PurchasedItem.ItemName = ItemName;
	PurchasedItem.ItemCount = Quantity;

	const int32 AddedSlotIndex = AddItemInPlayerStageInventory(PurchasedItem);
	if (AddedSlotIndex == INDEX_NONE) return SendResult(false, INDEX_NONE);

	// all passed : pay for the complete quantity
	Info.GoldStorage -= TotalPrice;
	OnPlayerInfoChanged.Broadcast();

	SendResult(true, AddedSlotIndex);
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

	}
}

void APlayerCharacterState::ApplyPlayerStageInventoryToPlayerStorage()
{
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
		// replicated with bIsDead in the same PlayerState update (ForceNetUpdate below)
		UpdateSurviveTime();
	}
	
	OnHealthChanged.Broadcast(OldValue, CurrentHealth, MaxHealth);
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

void APlayerCharacterState::OnRep_MyRank()
{
	OnPlayerRankChanged.Broadcast();
}

void APlayerCharacterState::OnRep_PlayerCombatInfo()
{
	OnPlayerCombatInfoChanged.Broadcast();
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
		if (InItemName.ItemName.IsNone() || InItemName.ItemCount <= 0) return INDEX_NONE;

		int32 ChangedSlotIndex = INDEX_NONE;

		int32 FoundIndex = PlayerStageInventory.IndexOfByPredicate(
			[&InItemName](const FInventoryItemData& StorageItem)
			{
				return StorageItem.ItemName == InItemName.ItemName
					&& StorageItem.ItemName != NAME_None; // itemName = None : defence code
			});

		if (FoundIndex != INDEX_NONE) // find index
		{
			const int32 ExistingCount = PlayerStageInventory[FoundIndex].ItemCount;
			if (ExistingCount < 0 || InItemName.ItemCount > MAX_int32 - ExistingCount) return INDEX_NONE;

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

void APlayerCharacterState::SetMyRank(const int32 InRank)
{
	if (!HasAuthority()) return;

	// same value = engine does not replicate it, check only to skip host HUD broadcast
	const uint8 NewRank = static_cast<uint8>(FMath::Clamp(InRank, 0, MAX_uint8));
	if (MyRank == NewRank) return;

	MyRank = NewRank;
	OnPlayerRankChanged.Broadcast(); // listen server host HUD (no OnRep on server)
}

const FCollectCombatInfo& APlayerCharacterState::GetPlayerCombatInfo() const
{
	return PlayerCombatInfo;
}

void APlayerCharacterState::UpdateSurviveTime()
{
	if (HasAuthority())
	{
		ATestGameState* GS = GetWorld()->GetGameState<ATestGameState>();
		if (!IsValid(GS)) return;

		if (PlayerCombatInfo.StageStartServerTime <= 0.0) return;

		PlayerCombatInfo.SurviveTime = static_cast<float>(
			GS->GetServerWorldTimeSeconds() - PlayerCombatInfo.StageStartServerTime);

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
	const int32 OldScore = PlayerCombatInfo.Score;

	PlayerCombatInfo.Score =
	FMath::RoundToInt(PlayerCombatInfo.DamageDealt * 2.f) -
	FMath::RoundToInt(PlayerCombatInfo.DamageTaken * 1.f) +
	(PlayerCombatInfo.KillCount * 5) +
	(PlayerCombatInfo.LootCount * 30) +
	FMath::RoundToInt(PlayerCombatInfo.SurviveTime * 1.5f);

	if (OldScore != PlayerCombatInfo.Score)
	{
		OnPlayerCombatInfoChanged.Broadcast(); // listen server host HUD (no OnRep on server)
	}
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
#if UE_BUILD_SHIPPING
	return;
#endif
	PlayerInfo.GoldStorage = SetGold;
}

//서버 즉시갱신 이벤트 - 성윤
void APlayerCharacterState::Server_SetStageGold_Implementation(int32 SetGold)
{
#if UE_BUILD_SHIPPING
	return;
#endif
	const int32 OldGold = PlayerStageGold;

	PlayerStageGold += SetGold;

	UE_LOG(LogTemp, Warning, TEXT("SERVER StageGold Old=%d New=%d"), OldGold, PlayerStageGold);

	OnPlayerStageGoldChanged.Broadcast(OldGold, PlayerStageGold);
	ForceNetUpdate();
}
#pragma endregion

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
