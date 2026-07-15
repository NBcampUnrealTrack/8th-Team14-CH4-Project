#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Item/CoinBase.h"
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
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|Stat")
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
	Patrol
==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Patrol")
	float PatrolRadius = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Patrol")
	float PatrolWaitTimeMin = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Patrol")
	float PatrolWaitTimeMax = 3.0f;

	/*==========================
		Reward
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	int32 ExpReward = 10;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	float DropChance = 20.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Info")
	FName ZombieName = "Zombie";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drop")
	int32 DropCoinValue = 1;

	/*==========================
		Animation
	==========================*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Animation")
	UAnimMontage* AttackMontage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Animation")
	UAnimMontage* DeathMontage;

	/*==========================
		State
	==========================*/
	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|State")
	bool bIsDead = false;

	FORCEINLINE bool IsDead() const { return bIsDead; }

	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Zombie|State")
	void OnZombieDeath();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAttackMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayDeathMontage();

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, Category = "Drop")
	TSubclassOf<ACoinBase> CoinClass;

protected:
	void Die();

	UFUNCTION()
	void OnRep_IsDead();

private:
	UFUNCTION()
	void DestroyZombie();

	// ���������� �������� �� ��Ʈ�ѷ� ����
	UPROPERTY()
	AController* LastDamageInstigator = nullptr;
};
