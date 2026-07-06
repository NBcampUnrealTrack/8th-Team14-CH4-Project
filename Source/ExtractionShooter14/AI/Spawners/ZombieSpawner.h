#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieSpawner.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API AZombieSpawner : public AActor
{
	GENERATED_BODY()

public:
	AZombieSpawner();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<TSubclassOf<ACharacter>> ZombieClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	float RespawnDelay = 5.0f;

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SpawnZombie();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	ACharacter* SpawnedZombie;

private:
	FTimerHandle RespawnTimerHandle;
	FTimerHandle CheckDeathTimerHandle;

	void CheckZombieDeath();
	void ScheduleRespawn();
};