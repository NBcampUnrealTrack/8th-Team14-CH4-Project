#include "TeleportTriggerBox_Base.h"

#include "Character/MyPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

ATeleportTriggerBox_Base::ATeleportTriggerBox_Base()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_C"));
	SphereComponent->SetupAttachment(StaticMeshComponent);
	SphereComponent->SetSphereRadius(300.f);
	
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::MoveToLocationBeginOverlap);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::MoveToLocationEndOverlap);
}

void ATeleportTriggerBox_Base::MoveToLocationBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	
	APawn* PlayerPawn = Cast<APawn>(OtherActor);
	if (!IsValid(PlayerPawn)) return;
	
	AMyPlayerController* PlayerControllerES = Cast<AMyPlayerController>(PlayerPawn->GetController());
	if (!IsValid(PlayerControllerES)) return;
	
	PlayerPawn->TeleportTo(MoveToLocation, MoveToRotation);
}

void ATeleportTriggerBox_Base::MoveToLocationEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}