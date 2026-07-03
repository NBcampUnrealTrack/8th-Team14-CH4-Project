#include "PlayerCharacterState.h"
#include "TestGameMode.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PlayerCharacter.h"

APlayerCharacterState::APlayerCharacterState()
	:bIsReady(false)
	,bIsDead(false)
{
}

void APlayerCharacterState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, PlayerInfo);
	DOREPLIFETIME(ThisClass, bIsReady);
	DOREPLIFETIME(ThisClass, bIsDead);
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

void APlayerCharacterState::Server_SetPlayerReady_Implementation(bool bReady)
{
	// in server
	SetPlayerReady(bReady);
	
	ATestGameMode* GameModeBaseES = Cast<ATestGameMode>(GetWorld()->GetAuthGameMode());
	if (!IsValid(GameModeBaseES)) return;
	
	GameModeBaseES->HandleAllPlayerisReady();
	
}

// =========================================================== MainPlayerCharacterState



void APlayerCharacterState::OnRep_Health()
{
}

void APlayerCharacterState::ApplyDamage(float DamageAmount)
{// is in server
	if (!HasAuthority()) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

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
	CurrentHealth = MaxHealth;
	SetbIsDead(false);
	SetPlayerReady(false);
}

void APlayerCharacterState::OnRep_bIsReady()
{
	OnPlayerReadyChanged.Broadcast(bIsReady);
}

void APlayerCharacterState::OnRep_bIsDead()
{
	OnPlayerbIsDeadChanged.Broadcast(bIsDead);
}

void APlayerCharacterState::SetPlayerReady(bool SetReady)
{
	if (bIsReady == SetReady)
		return;

	bIsReady = SetReady;

	OnRep_bIsReady();
	ForceNetUpdate();
}

void APlayerCharacterState::SetbIsDead(bool bIsDeadd)
{
	if (bIsDead == bIsDeadd)
		return;

	bIsDead = bIsDeadd;

	OnRep_bIsDead(); // 서버 즉시 처리
	ForceNetUpdate();
}