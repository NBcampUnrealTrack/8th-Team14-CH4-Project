#include "ZombieSpawner.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "GameFramework/Character.h"

AZombieSpawner::AZombieSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AZombieSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	SpawnZombie();

	GetWorldTimerManager().SetTimer(
		CheckDeathTimerHandle,
		this,
		&AZombieSpawner::CheckZombieDeath,
		0.5f,
		true);
}

void AZombieSpawner::SpawnZombie()
{
	if (!HasAuthority())
	{
		return;
	}

	if (ZombieClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ZombieSpawner] ZombieClasses is empty!"));
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, ZombieClasses.Num() - 1);
	TSubclassOf<ACharacter> SelectedClass = ZombieClasses[RandomIndex];

	if (!SelectedClass)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	SpawnedZombie = GetWorld()->SpawnActor<ACharacter>(
		SelectedClass,
		GetActorLocation(),
		GetActorRotation(),
		SpawnParams);

	if (SpawnedZombie)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ZombieSpawner] Zombie Spawned: %s"),
			*SpawnedZombie->GetName());
	}
}

void AZombieSpawner::CheckZombieDeath()
{
	if (!SpawnedZombie)
	{
		ScheduleRespawn();
		return;
	}

	AZombieCharacter* Zombie = Cast<AZombieCharacter>(SpawnedZombie);
	if (Zombie && Zombie->IsDead())
	{
		SpawnedZombie = nullptr;
		ScheduleRespawn();
	}
}

void AZombieSpawner::ScheduleRespawn()
{
	if (GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[ZombieSpawner] Respawn scheduled in %.1f seconds"),
		RespawnDelay);

	GetWorldTimerManager().SetTimer(
		RespawnTimerHandle,
		this,
		&AZombieSpawner::SpawnZombie,
		RespawnDelay,
		false);
}

