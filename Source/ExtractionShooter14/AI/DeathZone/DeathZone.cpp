#include "DeathZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Character/PlayerCharacter.h"
#include "Character/PlayerCharacterState.h"
#include "Engine/DamageEvents.h"

ADeathZone::ADeathZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	DeathZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DeathZoneBox"));
	RootComponent = DeathZoneBox;

	DeathZoneBox->SetBoxExtent(FVector(500.f, 500.f, 100.f));
	DeathZoneBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DeathZoneBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	DeathZoneBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ADeathZone::BeginPlay()
{
	Super::BeginPlay();

	DeathZoneBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ADeathZone::OnOverlapBegin);
}

void ADeathZone::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor)
	{
		return;
	}

	// 좀비인 경우 즉사
	AZombieCharacter* Zombie = Cast<AZombieCharacter>(OtherActor);
	if (Zombie && !Zombie->IsDead())
	{
		UGameplayStatics::ApplyDamage(
			Zombie,
			99999.f,
			nullptr,
			this,
			UDamageType::StaticClass());
		return;
	}

	// 플레이어인 경우 PS->TakenDamage로 처리 (위젯 흐름 유지)
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player)
	{
		return;
	}

	APlayerCharacterState* PS = Player->GetPlayerState<APlayerCharacterState>();
	AController* PlayerController = Player->GetController();

	if (!IsValid(PS) || !IsValid(PlayerController))
	{
		return;
	}

	// 이미 죽어있으면 처리 안함
	if (PS->GetbIsDead())
	{
		return;
	}

	FDamageEvent DamageEvent;
	PS->TakenDamage(
		99999.f,
		DamageEvent,
		PlayerController,
		this);
}

