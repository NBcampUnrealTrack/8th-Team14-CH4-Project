#include "PlayerCharacterState.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
{
}

void APlayerCharacterState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInfo);
	DOREPLIFETIME(ThisClass, bIsReady);
}

void APlayerCharacterState::ApplySavedDataToPlayer(FPlayerDatas ApplyPlayerData)
{
	// only initiate in server
	if (!HasAuthority()) return;
	
	PlayerInfo = ApplyPlayerData;
	OnPlayerDataChanged.Broadcast(PlayerInfo);
}

const FPlayerDatas& APlayerCharacterState::GetPlayerInfo() const
{
	return PlayerInfo;
}

void APlayerCharacterState::OnRep_PlayerInfo()
{
	//todo:: ui, inventory update
	UKismetSystemLibrary::PrintString
	(
		GetWorld(),
		FString::Printf(TEXT("Loaded PlayerData::Coin = %d"), PlayerInfo.Gold),
		true,
		true,
		FColor::Red,
		5.f
		);
	
	OnPlayerDataChanged.Broadcast(PlayerInfo);
}

void APlayerCharacterState::OnRep_bIsReady()
{
	OnPlayerReadyChanged.Broadcast(bIsReady);
}

void APlayerCharacterState::Server_SetPlayerReady_Implementation(bool bReady)
{
	bIsReady = bReady;
}

#pragma region Cheat
void APlayerCharacterState::PlusPlayerCoin_Implementation()
{
	PlayerInfo.Gold -= 100;
}
#pragma endregion 
