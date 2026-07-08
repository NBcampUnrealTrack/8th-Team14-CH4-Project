#include "ZombieCharacter.h"

#include "AI/Controllers/ZombieAIController.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Replication
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
	// 데미지는 서버만 처리
	if (!HasAuthority())
	{
		return 0.f;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ZombieCharacter] TakeDamage Called! Amount: %.1f"), DamageAmount);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, 5.f, FColor::Cyan,
			FString::Printf(TEXT("TakeDamage Called! %.1f"), DamageAmount));
	}

	const float ActualDamage = Super::TakeDamage(
		DamageAmount,
		DamageEvent,
		EventInstigator,
		DamageCauser);

	if (bIsDead)
	{
		return ActualDamage;
	}

	CurrentHP = FMath::Clamp(
		CurrentHP - ActualDamage,
		0.f,
		MaxHP);

	// 데미지 로그 출력
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

	if (CurrentHP <= 0.f)
	{
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

	// OnRep에서는 Multicast를 다시 호출하지 않는다.
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

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->UnPossess();
	}

	GetCapsuleComponent()->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);

	GetCharacterMovement()->DisableMovement();

	// 서버가 모든 클라이언트에게 사망 몽타주 재생
	Multicast_PlayDeathMontage();

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

