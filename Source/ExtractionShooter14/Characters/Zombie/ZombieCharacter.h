#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "ZombieCharacter.generated.h"

UCLASS()
class EXTRACTIONSHOOTER14_API AZombieCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AZombieCharacter();
protected:
	virtual void BeginPlay() override;
public:
	virtual void Tick(float DeltaTime) override;

	/*==========================
		Zombie Stat
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float MaxHP = 100.0f;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|Stat")
	float CurrentHP = 100.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float AttackDamage = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float MoveSpeed = 250.0f;

	/*==========================
		Combat
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float DetectRange = 1000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float AttackRange = 150.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float AttackCooldown = 1.5f;

	/*==========================
		Reward
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	int32 ExpReward = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	float DropChance = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Info")
	FName ZombieName = "Zombie";

	/*==========================
		Animation
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Animation")
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Animation")
	UAnimMontage* DeathMontage;

	/*==========================
		Damage / Death
	==========================*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|State")
	bool bIsDead = false;

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie|State")
	void OnZombieDeath();

protected:
	void Die();

private:
	UFUNCTION()
	void DestroyZombie();
};
