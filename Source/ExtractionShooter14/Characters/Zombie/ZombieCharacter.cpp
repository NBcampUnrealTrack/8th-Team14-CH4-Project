#include "ZombieCharacter.h"
#include "AI/Controllers/ZombieAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "AIController.h"

AZombieCharacter::AZombieCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AZombieAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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

float AZombieCharacter::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (bIsDead)
	{
		return ActualDamage;
	}

	CurrentHP = FMath::Clamp(CurrentHP - ActualDamage, 0.0f, MaxHP);

	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AZombieCharacter::Die()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	// AI Á¤Áö
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->UnPossess();
	}

	// Ãæµ¹ ²ô±â
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// »ç¸Á ¸ùÅ¸ÁÖ Àç»ý
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	OnZombieDeath();
}

