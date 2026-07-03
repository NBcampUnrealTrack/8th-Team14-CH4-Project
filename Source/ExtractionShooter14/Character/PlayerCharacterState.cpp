#include "PlayerCharacterState.h"
#include "TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
{
}

void APlayerCharacterState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInfo);
	DOREPLIFETIME(ThisClass, bIsReady);
	DOREPLIFETIME(ThisClass, CurrentHealth);
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

void APlayerCharacterState::SetPlayerReady(bool SetReady)
{
	// init only server
	bIsReady = SetReady;
}

void APlayerCharacterState::Server_SetPlayerReady_Implementation(bool bReady)
{
	// in server
	bIsReady = bReady;
	OnRep_bIsReady();
	
	ATestGameMode* GameModeBaseES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeBaseES)) return;
	
	GameModeBaseES->HandleAllPlayerisReady();
	
}

#pragma region Cheat
void APlayerCharacterState::PlusPlayerCoin_Implementation()
{
	PlayerInfo.Gold -= 100;
}
#pragma endregion 

// =========================================================== MainPlayerCharacterState

void APlayerCharacterState::OnRep_Health()
{
}

void APlayerCharacterState::ApplyDamage(float DamageAmount)
{
	if (!HasAuthority()) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
		{
			PlayerCharacter->Die();
		}
	}
}