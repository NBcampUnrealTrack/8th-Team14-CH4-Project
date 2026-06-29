#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
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

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float MaxHP = 100.0f;

	// 현재 체력
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Zombie|Stat")
	float CurrentHP = 100.0f;

	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float AttackDamage = 10.0f;

	// 이동속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stat")
	float MoveSpeed = 250.0f;

	/*==========================
		Combat
	==========================*/

	// 플레이어 감지 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float DetectRange = 1000.0f;

	// 공격 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float AttackRange = 150.0f;

	// 공격 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Combat")
	float AttackCooldown = 1.5f;

	/*==========================
		Reward
	==========================*/

	// 처치 시 경험치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	int32 ExpReward = 10;

	// 드랍 확률
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Reward")
	float DropChance = 20.0f;

	// 좀비 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Info")
	FName ZombieName = "Zombie";

	// 사용할 Behavior Tree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Zombie|AI")
	UBehaviorTree* BehaviorTree;
};
