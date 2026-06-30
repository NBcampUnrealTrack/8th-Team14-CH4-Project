#include "TeleportTriggerBox_Base.h"

#include "Character/MyPlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

ATeleportTriggerBox_Base::ATeleportTriggerBox_Base()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);
	
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(StaticMeshComponent);
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::MoveToLocationBeginOverlap);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::MoveToLocationEndOverlap);
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