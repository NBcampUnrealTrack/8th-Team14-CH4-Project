#include "DeathZone.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/Zombie/ZombieCharacter.h"
#include "Character/PlayerCharacter.h"

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

	// 좀비인 경우 TakeDamage로 즉사
	AZombieCharacter* Zombie = Cast<AZombieCharacter>(OtherActor);
	if (Zombie)
	{
		UGameplayStatics::ApplyDamage(
			Zombie,
			99999.f,
			nullptr,
			this,
			UDamageType::StaticClass());
		return;
	}

	// 플레이어인 경우 Die() 직접 호출
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player)
	{
		Player->Die();
	}
}

