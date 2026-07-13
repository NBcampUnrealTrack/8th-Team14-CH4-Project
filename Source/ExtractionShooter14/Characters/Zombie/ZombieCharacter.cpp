#include "ZombieCharacter.h"
#include "AI/Controllers/ZombieAIController.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/PlayerCharacterState.h"
#include "GameFramework/PlayerController.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bReplicates = true;
	SetReplicateMovement(true);
}

void AZombieCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHP = MaxHP;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void AZombieCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieCharacter::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AZombieCharacter, CurrentHP);
	DOREPLIFETIME(AZombieCharacter, bIsDead);
}

float AZombieCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.f;
	}

	if (bIsDead)
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	CurrentHP = FMath::Clamp(
		CurrentHP - ActualDamage,
		0.f,
		MaxHP);

	// 마지막 공격자 저장
	if (IsValid(EventInstigator))
	{
		LastDamageInstigator = EventInstigator;
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] TakeDamage! Damage: %.1f / HP: %.1f / %.1f"),
		*ZombieName.ToString(),
		ActualDamage,
		CurrentHP,
		MaxHP);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 2.f, FColor::Green,
			FString::Printf(TEXT("Server: HP : %.0f / %.0f"),
				CurrentHP,
				MaxHP));
	}
	
	APlayerCharacterState* PlayerES = EventInstigator->GetPlayerState<APlayerCharacterState>();
	if (IsValid(PlayerES))
	{
		PlayerES->AddPlayerCombatInfo_DamageDealt(ActualDamage);
	}
	if (CurrentHP <= 0.f)
	{
		if (IsValid(PlayerES))
		{
			PlayerES->AddPlayerCombatInfo_KillCount(1);
		}
		Die();
	}
	
	return ActualDamage;
}

void AZombieCharacter::Multicast_PlayAttackMontage_Implementation()
{
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void AZombieCharacter::Multicast_PlayDeathMontage_Implementation()
{
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void AZombieCharacter::OnRep_IsDead()
{
	if (!bIsDead)
	{
		return;
	}
	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}
}

void AZombieCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	// AI 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->UnPossess();
	}

	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	Multicast_PlayDeathMontage();

	// 마지막 공격자에게 골드 지급
	// if (IsValid(LastDamageInstigator))
	// {
	// 	APlayerCharacterState* KillerPS =
	// 		LastDamageInstigator->GetPlayerState<APlayerCharacterState>();
	//
	// 	if (IsValid(KillerPS))
	// 	{
	// 		// 드랍률 기준 100골드 중 드랍률% 만큼 지급
	// 		const int32 GoldReward = FMath::RoundToInt(DropChance);
	//
	// 		KillerPS->AddPlayerStageGold(GoldReward);
	//
	// 		// 플레이어 이름 가져오기
	// 		const FString PlayerName = KillerPS->GetPlayerName();
	//
	// 		UE_LOG(LogTemp, Warning, TEXT("[Reward] %s earned %d Gold!"),
	// 			*PlayerName, GoldReward);
	//
	// 		if (GEngine)
	// 		{
	// 			GEngine->AddOnScreenDebugMessage(
	// 				-1, 5.f, FColor::Yellow,
	// 				FString::Printf(TEXT("[%s] earned %d Gold!"),
	// 					*PlayerName, GoldReward));
	// 		}
	// 	}
	// }

	float DestroyDelay = 2.0f;
	if (DeathMontage)
	{
		DestroyDelay = DeathMontage->GetPlayLength();
	}

	FTimerHandle DestroyTimerHandle;
	GetWorldTimerManager().SetTimer(
		DestroyTimerHandle,
		this,
		&AZombieCharacter::DestroyZombie,
		DestroyDelay,
		false);

	OnZombieDeath();
}

void AZombieCharacter::DestroyZombie()
{
	if (HasAuthority())
	{
		Destroy();
	}
}