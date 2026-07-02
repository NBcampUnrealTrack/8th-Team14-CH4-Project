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

	// AI 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
		AIController->UnPossess();
	}

	// 충돌 끄기
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	// 사망 몽타주 재생 후 길이만큼 대기하다가 Destroy
	float DestroyDelay = 2.0f;
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
		DestroyDelay = DeathMontage->GetPlayLength();
	}

	// 몽타주 재생 시간 이후 액터 제거
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
	Destroy();
}

